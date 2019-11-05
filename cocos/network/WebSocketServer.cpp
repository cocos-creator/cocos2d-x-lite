/****************************************************************************
 Copyright (c) 2019 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You shall
 not use Cocos Creator software for developing other software or tools that's
 used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.

 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#include "base/ccConfig.h"

#if (USE_SOCKET > 0) && (USE_WEBSOCKET_SERVER > 0)

#include <iostream>
#include <cassert>

#include "WebSocketServer.h"
#include "platform/CCApplication.h"
#include "base/CCScheduler.h"
namespace cocos2d {
namespace network {

#define RUN_IN_GAMETHREAD(task) \
    do { \
        cocos2d::Application::getInstance()->getScheduler()->performFunctionInCocosThread([=]() { \
        task; \
    });\
    } while(0)

#define DISPATCH_CALLBACK_IN_GAMETHREAD() do {\
        data->setCallback([callback](const std::string& msg) { \
            auto wrapper = [callback, msg]() {callback(msg); }; \
            RUN_IN_GAMETHREAD(wrapper()); \
        }); \
    }while(0)

#define RUN_IN_SERVERTHREAD(task) do {      \
        schedule_async_task(&_async, [=](){ \
            task;                           \
        });                                 \
    } while(0)

//#define LOGE() CCLOG("WSS: %s", __FUNCTION__)
#define LOGE()


#define MAX_MSG_PAYLOAD 2048
#define SEND_BUFF 1024

struct AsyncTaskData {
    std::mutex mtx;
    std::list<std::function<void()> > tasks;
};

// only run in server loop
static void scheduler_task_cb(uv_async_t* asyn)
{
    AsyncTaskData* data = (AsyncTaskData*)asyn->data;
    std::lock_guard<std::mutex> guard(data->mtx);
    while (!data->tasks.empty())
    {
        data->tasks.front()();
        data->tasks.pop_front();
    }

}
static void async_init(uv_loop_t* loop, uv_async_t* async)
{
    memset(async, 0, sizeof(uv_async_t));
    uv_async_init(loop, async, scheduler_task_cb);
    async->data = new AsyncTaskData();
}

// run in game thread, dispatch runnable object into server loop
static void schedule_async_task(uv_async_t* asyn, std::function<void()> func)
{

    AsyncTaskData* data = (AsyncTaskData*)asyn->data;
    {
        std::lock_guard<std::mutex> guard(data->mtx);
        data->tasks.emplace_back(func);
    }
    uv_async_send(asyn);
}

DataFrag::DataFrag(const std::string& data) :_isBinary(false)
{
    _underlying_data.resize(data.size() + LWS_PRE);
    memcpy(getData(), data.c_str(), data.length());
}

DataFrag::DataFrag(const void* data, int len, bool isBinary) :_isBinary(isBinary)
{
    _underlying_data.resize(len + LWS_PRE);
    memcpy(getData(), data, len);
}

DataFrag::~DataFrag()
{
}

void DataFrag::append(unsigned char* p, int len)
{
    //_underlying_data.emplace_back(_underlying_data.end(), p, len);
    _underlying_data.insert(_underlying_data.end(), p, p + len);
}

int DataFrag::slice(unsigned char** p, int len) {
    *p = getData() + _consumed;
    if (_consumed + len > size()) {
        return size() - _consumed;
    }
    return len;
}

int DataFrag::consume(int len) {
    _consumed = len + _consumed > size() ? size() : len + _consumed;
    return _consumed;
}

int DataFrag::remain() const {
    return size() - _consumed;
}


std::string DataFrag::toString()
{
    return std::string((char*)getData(), size());
}

//static int websocket_server_callback(struct lws* wsi, enum lws_callback_reasons reason,
//    void* user, void* in, size_t len);

static struct lws_protocols protocols[] = {
    {
        "", //protocol name
        WebSocketServer::websocket_server_callback,
        sizeof(int),
        MAX_MSG_PAYLOAD
    },
    {
        nullptr, nullptr, 0
    }
};

static const struct lws_extension exts[] = {
    {
        "permessage-deflate",
        lws_extension_callback_pm_deflate,
        "permessage-deflate; client_on_context_takeover; client_max_window_bits"
    },
    {
        "deflate-frame",
        lws_extension_callback_pm_deflate,
        "deflate-frame"
    },
    {
        nullptr, nullptr, nullptr
    }
};


WebSocketServer::~WebSocketServer()
{

    if (_ctx) {
        lws_context_destroy(_ctx);
        lws_context_destroy2(_ctx);
        _ctx = nullptr;
    }
    if (_async.data) {
        delete (AsyncTaskData*)_async.data;
    }
}

bool WebSocketServer::close(std::function<void(const std::string & errorMsg)> callback)
{
    _onclose_cb = callback;
    if(_ctx)
        lws_libuv_stop(_ctx);
    return true;
}

bool WebSocketServer::closeAsync(std::function<void(const std::string & errorMsg)> callback)
{
    if (_serverState != ServerThreadState::RUNNING) {
        return false;
    }
    RUN_IN_SERVERTHREAD(this->close(callback));
    return true;
}


bool WebSocketServer::listen(std::shared_ptr<WebSocketServer> server, int port, const std::string& host, std::function<void(const std::string & errorMsg)> callback)
{
    server->_serverState = ServerThreadState::RUNNING;
    lws_set_log_level(-1, nullptr);

    if (server->_ctx) {
        if (callback) {
            RUN_IN_GAMETHREAD(callback("Error: lws_context already created!"));
        }
        RUN_IN_GAMETHREAD(if (server->_onerror) server->_onerror("websocket listen error!"));
        server->_serverState = ServerThreadState::ST_ERROR;
        return false;
    }

    server->_host = host;

    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port = port;
    info.iface = server->_host.empty() ? nullptr : server->_host.c_str();
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    info.extensions = exts;
    info.options = LWS_SERVER_OPTION_VALIDATE_UTF8
        |LWS_SERVER_OPTION_LIBUV
        |LWS_SERVER_OPTION_SKIP_SERVER_CANONICAL_NAME
        ;
    info.timeout_secs = 60; //
    info.max_http_header_pool = 1;
    info.user = server.get();


    server->_ctx = lws_create_context(&info);

    if (!server->_ctx) {
        if (callback) {
            RUN_IN_GAMETHREAD(callback("Error: Failed to create lws_context!"));
        }
        RUN_IN_GAMETHREAD(if (server->_onerror)server->_onerror("websocket listen error!"));
        server->_serverState = ServerThreadState::ST_ERROR;
        return false;
    }
    uv_loop_t* loop = nullptr;
    if (lws_uv_initloop(server->_ctx, loop, 0)) {
        if (callback) {
            RUN_IN_GAMETHREAD(callback("Error: Failed to create libuv loop!"));
        }
        RUN_IN_GAMETHREAD(if (server->_onerror)server->_onerror("websocket listen error, failed to create libuv loop!"));
        server->_serverState = ServerThreadState::ST_ERROR;
        return false;
    }

    loop = lws_uv_getloop(server->_ctx, 0);
    async_init(loop, &server->_async);
    RUN_IN_GAMETHREAD(if(server->_onlistening)server->_onlistening(""));
    RUN_IN_GAMETHREAD(if(server->_onbegin)server->_onbegin());
    RUN_IN_GAMETHREAD(if(callback)callback(""));

    lws_libuv_run(server->_ctx, 0);
    uv_close((uv_handle_t*)&server->_async, nullptr);

    RUN_IN_GAMETHREAD(if(server->_onclose)server->_onclose(""));
    RUN_IN_GAMETHREAD(if(server->_onclose_cb)server->_onclose_cb(""));
    RUN_IN_GAMETHREAD(if(server->_onend)server->_onend());
    server->_serverState = ServerThreadState::STOPPED;

    return true;
}

bool WebSocketServer::listenAsync(std::shared_ptr<WebSocketServer> server,int port, const std::string& host, std::function<void(const std::string & errorMsg)> callback)
{
    if (server->_serverState != ServerThreadState::NOT_BOOTED) {
        return false;
    }

    std::thread([=]() {
        WebSocketServer::listen(server, port, host, callback);
        }).detach();

    return true;
}


std::vector<std::shared_ptr<WSServerConnection>> WebSocketServer::getConnections() const
{
    std::vector<std::shared_ptr<WSServerConnection> > ret;
    for (auto itr : _conns) {
        ret.emplace_back(itr.second);
    }
    return ret;
}

void WebSocketServer::onCreateClient(struct lws* wsi)
{
    LOGE();
    std::shared_ptr<WSServerConnection> conn = std::make_shared<WSServerConnection>(wsi);
    //char ip[221] = { 0 };
    //char addr[221] = { 0 };
    //lws_get_peer_addresses(wsi, lws_get_socket_fd(wsi), ip, 220, addr, 220);
    //lws_get_peer_simple(wsi, ip, 220);
    _conns.emplace(wsi, conn);
    RUN_IN_GAMETHREAD(if(_onconnection) _onconnection(conn));
    conn->onConnected();
}

void WebSocketServer::onDestroyClient(struct lws* wsi)
{
    LOGE();
    auto itr = _conns.find(wsi);
    if (itr != _conns.end()) {
        itr->second->finallyClosed();
        _conns.erase(itr);
    }
}
void WebSocketServer::onCloseClient(struct lws* wsi)
{
    LOGE();
    auto itr = _conns.find(wsi);
    if (itr != _conns.end()) {
        itr->second->setClosed();
    }
}

void WebSocketServer::onCloseClientInit(struct lws* wsi, void* in, int len)
{
    LOGE();
    int16_t code;
    char* msg = nullptr;

    auto itr = _conns.find(wsi);
    if (itr != _conns.end()) {
        if (len > 2) {
            code = ntohs(*(int16_t*)in);
            msg = (char*)in + sizeof(code);
            std::string cp(msg, len - sizeof(code));
            itr->second->onCloseInit(code, cp);
        }
    }
}

void WebSocketServer::onClientReceive(struct lws* wsi, void* in, int len)
{
    LOGE();
    auto itr = _conns.find(wsi);
    if (itr != _conns.end()) {
        itr->second->onDataReceive(in, len);
    }
}
int WebSocketServer::onClientWritable(struct lws* wsi)
{
    LOGE();
    auto itr = _conns.find(wsi);
    if (itr != _conns.end()) {
        return itr->second->onDrainData();
    }
    return 0;
}

void WebSocketServer::onClientHTTP(struct lws* wsi)
{
    LOGE();
    auto itr = _conns.find(wsi);
    if (itr != _conns.end()) {
        itr->second->onHTTP();
    }
}

WSServerConnection::WSServerConnection(struct lws* wsi) : _wsi(wsi)
{
    LOGE();
    uv_loop_t* loop = lws_uv_getloop(lws_get_context(wsi), 0);
    async_init(loop, &_async);
}


WSServerConnection::~WSServerConnection()
{
    LOGE();
    if (_async.data) {
        delete (AsyncTaskData*)_async.data;
    }
}

bool WSServerConnection::send(std::shared_ptr<DataFrag> data)
{
    _sendQueue.emplace_back(data);
    onDrainData();
    return true;
}


bool WSServerConnection::sendText(const std::string& text, std::function<void(const std::string&)> callback)
{
    LOGE();
    std::shared_ptr<DataFrag> data = std::make_shared<DataFrag>(text);
    if (callback) {
        DISPATCH_CALLBACK_IN_GAMETHREAD();
    }
    send(data);
    return true;
}

bool WSServerConnection::sendTextAsync(const std::string& text, std::function<void(const std::string&)> callback)
{
    LOGE();
    std::shared_ptr<DataFrag> data = std::make_shared<DataFrag>(text);
    if (callback) {
        DISPATCH_CALLBACK_IN_GAMETHREAD();
    }
    RUN_IN_SERVERTHREAD(this->send(data));

    return true;
}

bool WSServerConnection::sendBinary(const void* in, size_t len, std::function<void(const std::string&)> callback)
{
    LOGE();
    std::shared_ptr<DataFrag> data = std::make_shared<DataFrag>(in, len);
    if (callback) {
        DISPATCH_CALLBACK_IN_GAMETHREAD();
    }
    send(data);
    return true;
}


bool WSServerConnection::sendBinaryAsync(const void* in, size_t len, std::function<void(const std::string&)> callback)
{
    LOGE();
    std::shared_ptr<DataFrag> data = std::make_shared<DataFrag>(in, len);
    if (callback) {
        DISPATCH_CALLBACK_IN_GAMETHREAD();
    }
    RUN_IN_SERVERTHREAD(this->send(data));

    return true;
}


bool WSServerConnection::close(int code, std::string message)
{
    LOGE();
    if (!_wsi) return false;
    _readyState = ReadyState::CLOSING;
    _closeReason = message;
    _closeCode = code;
    setClosed();
    lws_callback_on_writable(_wsi);
    return true;
}


bool WSServerConnection::closeAsync(int code, std::string message)
{
    LOGE();
    _readyState = ReadyState::CLOSING;
    if (!_wsi) return false;
    RUN_IN_SERVERTHREAD(this->close(code, message));
    return true;
}

void WSServerConnection::onConnected()
{
    _readyState = ReadyState::OPEN;
    RUN_IN_GAMETHREAD(if(_onconnect)_onconnect());
}

void WSServerConnection::onDataReceive(void* in, int len)
{
    LOGE();

    bool isFinal = lws_is_final_fragment(_wsi);
    bool isBinary = lws_frame_is_binary(_wsi);

    if (!_prevPkg) {
        _prevPkg = std::make_shared<DataFrag>(in, len, isBinary);
    }
    else {
        _prevPkg->append((unsigned char*)in, len);
    }

    if (isFinal) {
        //trigger event
        std::shared_ptr<DataFrag> fullpkg = _prevPkg;
        if (isBinary)
        {
            RUN_IN_GAMETHREAD(if(_onbinary)_onbinary(fullpkg));
        }
        if (!isBinary)
        {
            RUN_IN_GAMETHREAD(if(_ontext)_ontext(fullpkg));
        }

        RUN_IN_GAMETHREAD(if(_ondata)_ondata(fullpkg));
        
        _prevPkg.reset();
    }
}



int WSServerConnection::onDrainData()
{
    LOGE();
    if (!_wsi) return -1;
    if (_closed) return -1;
    if (_readyState == ReadyState::CLOSING) {
        return -1;
    }
    if (_readyState != ReadyState::OPEN) return 0;
    unsigned char* p = nullptr;
    int send_len = 0;
    int finish_len = 0;
    int flags = 0;

    std::vector<char> buff(SEND_BUFF + LWS_PRE);

    if (!_sendQueue.empty())
    {
        std::shared_ptr<DataFrag> frag = _sendQueue.front();

        send_len = frag->slice(&p, SEND_BUFF);

        if(frag->isFront())
        {
            if (frag->isBinary()) 
            {
                flags |= LWS_WRITE_BINARY;
            }
            if (frag->isString())  
            {
                flags |= LWS_WRITE_TEXT;
            }
        }

        if (frag->remain() != send_len)
        {
            // remain bytes > 0
            // not FIN
            flags |= LWS_WRITE_NO_FIN;
        }

        if (!frag->isFront()) {
            flags |= LWS_WRITE_CONTINUATION;
        }

        finish_len = lws_write(_wsi, p, send_len, (lws_write_protocol)flags);

        if (finish_len == 0)
        {
            frag->onFinish("Connection Closed");
            return -1;
        }
        else if (finish_len < 0)
        {
            frag->onFinish("Send Error!");
            return -1; 
        }
        else
        {
            frag->consume(finish_len);
        }


        if (frag->remain() == 0) {
            frag->onFinish("");
            _sendQueue.pop_front();
        }
        lws_callback_on_writable(_wsi);
    }

    return 0;

}

void WSServerConnection::onHTTP()
{
    if (!_wsi) return;

    _headers.clear();

    _readyState = ReadyState::CONNECTING;

    int n = 0, len;
    std::vector<char> buf(256);
    const char* c;
    do {

        lws_token_indexes idx = static_cast<lws_token_indexes>(n);
        c = (const char*)lws_token_to_string(idx);
        if (!c) {
            n++;
            continue;
        }
        len = lws_hdr_total_length(_wsi, idx);
        if (!len) {
            n++;
            continue;
        }
        else if (len + 1 > buf.size())
        {
            buf.resize(len + 1);
        }
        lws_hdr_copy(_wsi, buf.data(), buf.size(), idx);
        buf[len + 1] = '\0';
        _headers.emplace(std::string(c), std::string(buf.data()));
        n++;
    } while (c);

}

void WSServerConnection::onCloseInit(int code, const std::string& msg)
{
    _closeCode = code;
    _closeReason = msg;
}

void WSServerConnection::setClosed()
{
    if (_closed) return;
    lws_close_reason(_wsi, (lws_close_status)_closeCode, (unsigned char*)_closeReason.c_str(), _closeReason.length());
    _closed = true;
}

void WSServerConnection::finallyClosed()
{
    _readyState = ReadyState::CLOSED;
    //on wsi destroied
    if (_wsi)
    {
        RUN_IN_GAMETHREAD(if(_onclose)_onclose(_closeCode, _closeReason));
        RUN_IN_GAMETHREAD(if(_onend) _onend());
    }
}


std::vector<std::string> WSServerConnection::getProtocols() {
    std::vector<std::string> ret;
    if (_wsi) {
        //TODO cause abort 
        //const struct lws_protocols* protos = lws_get_protocol(_wsi);
        //while (protos && protos->name != nullptr)
        //{
        //    ret.emplace_back(protos->name);
        //    protos++;
        //}
    }
    return ret;
}

std::map<std::string, std::string> WSServerConnection::getHeaders()
{
    if (!_wsi) return {};
    return _headers;
}


int WebSocketServer::websocket_server_callback(struct lws* wsi, enum lws_callback_reasons reason,
    void* user, void* in, size_t len)
{
    int ret = 0;
    WebSocketServer* server = nullptr;
    lws_context* ctx = nullptr;

    if (wsi)
        ctx = lws_get_context(wsi);
    if (ctx)
        server = static_cast<WebSocketServer*>(lws_context_user(ctx));

    if (!server) {
        return 0;
    }
    

    switch (reason)
    {
    case LWS_CALLBACK_ESTABLISHED:
        break;
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        break;
    case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH:
        break;
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        break;
    case LWS_CALLBACK_CLOSED:
        break;
    case LWS_CALLBACK_CLOSED_HTTP:
        break;
    case LWS_CALLBACK_RECEIVE:
        server->onClientReceive(wsi, in, len);
        break;
    case LWS_CALLBACK_RECEIVE_PONG:
        break;
    case LWS_CALLBACK_CLIENT_RECEIVE:
        server->onClientReceive(wsi, in, len);
        break;
    case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
        break;
    case LWS_CALLBACK_CLIENT_WRITEABLE:
        ret = server->onClientWritable(wsi);
        break;
    case LWS_CALLBACK_SERVER_WRITEABLE:
        ret = server->onClientWritable(wsi);
        break;
    case LWS_CALLBACK_HTTP:
        //server->onClientHTTP(wsi);
        break;
    case LWS_CALLBACK_HTTP_BODY:
        break;
    case LWS_CALLBACK_HTTP_BODY_COMPLETION:
        break;
    case LWS_CALLBACK_HTTP_FILE_COMPLETION:
        break;
    case LWS_CALLBACK_HTTP_WRITEABLE:
        break;
    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
        break;
    case LWS_CALLBACK_FILTER_HTTP_CONNECTION:
        break;
    case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
        break;
    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
        break;
    case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS:
        break;
    case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS:
        break;
    case LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION:
        break;
    case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
        break;
    case LWS_CALLBACK_CONFIRM_EXTENSION_OKAY:
        break;
    case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
        break;
    case LWS_CALLBACK_PROTOCOL_INIT:
        break;
    case LWS_CALLBACK_PROTOCOL_DESTROY:
        break;
    case LWS_CALLBACK_WSI_CREATE:
        server->onCreateClient(wsi);
        break;
    case LWS_CALLBACK_WSI_DESTROY:
        server->onDestroyClient(wsi);
        break;
    case LWS_CALLBACK_GET_THREAD_ID:
        break;
    case LWS_CALLBACK_ADD_POLL_FD:
        break;
    case LWS_CALLBACK_DEL_POLL_FD:
        break;
    case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
        break;
    case LWS_CALLBACK_LOCK_POLL:
        break;
    case LWS_CALLBACK_UNLOCK_POLL:
        break;
    case LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY:
        break;
    case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
        server->onCloseClientInit(wsi, in, len);
        break;
    case LWS_CALLBACK_WS_EXT_DEFAULTS:
        break;
    case LWS_CALLBACK_CGI:
        break;
    case LWS_CALLBACK_CGI_TERMINATED:
        break;
    case LWS_CALLBACK_CGI_STDIN_DATA:
        break;
    case LWS_CALLBACK_CGI_STDIN_COMPLETED:
        break;
    case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP:
        break;
    case LWS_CALLBACK_CLOSED_CLIENT_HTTP:
        server->onCloseClient(wsi);
        break;
    case LWS_CALLBACK_RECEIVE_CLIENT_HTTP:
        break;
    case LWS_CALLBACK_COMPLETED_CLIENT_HTTP:
        break;
    case LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ:
        break;
    case LWS_CALLBACK_HTTP_BIND_PROTOCOL:
        break;
    case LWS_CALLBACK_HTTP_DROP_PROTOCOL:
        break;
    case LWS_CALLBACK_CHECK_ACCESS_RIGHTS:
        break;
    case LWS_CALLBACK_PROCESS_HTML:
        break;
    case LWS_CALLBACK_ADD_HEADERS:
        break;
    case LWS_CALLBACK_SESSION_INFO:
        break;
    case LWS_CALLBACK_GS_EVENT:
        break;
    case LWS_CALLBACK_HTTP_PMO:
        break;
    case LWS_CALLBACK_CLIENT_HTTP_WRITEABLE:
        break;
    case LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION:
        break;
    case LWS_CALLBACK_RAW_RX:
        break;
    case LWS_CALLBACK_RAW_CLOSE:
        break;
    case LWS_CALLBACK_RAW_WRITEABLE:
        break;
    case LWS_CALLBACK_RAW_ADOPT:
        break;
    case LWS_CALLBACK_RAW_ADOPT_FILE:
        break;
    case LWS_CALLBACK_RAW_RX_FILE:
        break;
    case LWS_CALLBACK_RAW_WRITEABLE_FILE:
        break;
    case LWS_CALLBACK_RAW_CLOSE_FILE:
        break;
    case LWS_CALLBACK_SSL_INFO:
        break;
    case LWS_CALLBACK_CHILD_WRITE_VIA_PARENT:
        break;
    case LWS_CALLBACK_CHILD_CLOSING:
        break;
    case LWS_CALLBACK_CGI_PROCESS_ATTACH:
        break;
    case LWS_CALLBACK_USER:
        break;
    default:
        break;
    }
    return ret;
}

} // namespace network
} // namespace cocos2d

#endif
