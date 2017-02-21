#include "config_parser.h"
#include "lightning_server.h"
#include "request_handlers.h"
#include "request_router.h"
#include "server_config.h"
#include "request.h"
#include "response.h"

#include <iostream>
#include <cstddef>

LightningServer::LightningServer(const NginxConfig config_)
  : server_config_(config_),
    io_service_(),
    acceptor_(io_service_)
{
  // We expect the port to be stored found
  // in the config with the following format:
  // server {
  //     ...
  //     listen $(PORT);
  //     ...
  // }
  std::vector<std::string> query = {"server", "listen"};
  server_config_.propertyLookUp(query, port_);
  std::cout << port_ << std::endl;
}

LightningServer::~LightningServer() {}

void LightningServer::start() {
  // Setup server to listen for TCP connection on config file specified port
  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), std::stoi(port_));
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
  acceptor_.listen();

  // Lightning listening loop
  for (;;) {
    // Accept connection request
    boost::asio::ip::tcp::socket socket(io_service_);
    acceptor_.accept(socket);

    // Read in request
    char request_buffer[MAX_REQ_SIZE];
    boost::system::error_code ec;
    std::size_t request_buffer_size = socket.read_some(boost::asio::buffer(request_buffer), ec);

    switch (ec.value()) {
      case boost::system::errc::success:
        std::cout << "~~~~~~~~~~Request~~~~~~~~~~\n" << request_buffer << std::endl;
        break;
      default:
        std::cout << "Error reading from socket, code: " << ec << std::endl;
        continue;
    }

    // DEBUG Testing Below; Can be removed later
    // Request Req;
    // std::unique_ptr<Request> req = Req.Parse(request_buffer);
    // std::cout << "Contents of Request Object:" + req->raw_request() << std::endl;
    // std::cout << "Method:" + req->method() << std::endl;
    // std::cout << "URI: " + req->uri() << std::endl;
    // std::cout << "Version: " + req->version() << std::endl;
    // for (size_t i = 0; i< req->headers().size(); i++) {
    //   std::cout << "Header Val: " + req->headers()[i].first + ", " + req->headers()[i].second << std::endl;
    // }

    // Response Resp;
    // Resp.SetStatus(Response::OK);

    // Handle echo response in external handler
    char* response_buffer = nullptr;
    size_t response_buffer_size  = 0;

    RequestRouter router;
    bool routingSuccess = router.routeRequest(server_config_,
                                              request_buffer,
                                              request_buffer_size,
                                              response_buffer,
                                              response_buffer_size);

    // TODO: Use-after-free vulnerability if response_buffer is used after
    // EchoRequestHandler is out of scope

    if (!routingSuccess && response_buffer_size == 0) {
      std::cout << "Failed to route due to invalid request\n";
      continue;
    }

    // Write back response
    boost::asio::write(socket,
                       boost::asio::buffer(response_buffer, response_buffer_size));
    delete response_buffer;
  }
}
