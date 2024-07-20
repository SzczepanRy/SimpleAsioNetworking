#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/network_v4.hpp>
#include <boost/asio/redirect_error.hpp>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <system_error>
#include <thread>
#include <vector>

#define ASIO_STANDALONE
#include <boost/asio.hpp>

using namespace boost;

std::vector<char> vBuffer(20 * 1024);

void grabSomeData(asio::ip::tcp::socket &socket) {
  socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),

                         [&](std::error_code ec, std::size_t length) {
                           if (!ec) {
                             std::cout << "\n\n READ " << length
                                       << " bytes \n\n";
                             for (int i = 0; i < length; i++) {
                               std::cout << vBuffer[i];
                             }

                             grabSomeData(socket);
                           }
                         });
}

int main() {
  // part 1

  std::cout << "START\n";

  system::error_code ec;

  // create a context
  asio::io_context ctx;

  // prevent the context from finishing
  asio::io_context::work idleWork(ctx);

  // start context thread
  //
  std::thread ctxT = std::thread([&]() { ctx.run(); });

  // get the addres of somewhere we wish to connect to
  // endpoint
  asio::ip::tcp::endpoint endpoint(asio::ip::make_address("93.184.216.34", ec),
                                   80);

  // create a socket, the ctx will deliver the inplementation
  asio::ip::tcp::socket socket(ctx);

  // try to connect to the specifiend address
  auto val = socket.connect(endpoint, ec);

  std::cout << "socket connection return value" << val << '\n';

  if (!ec) {
    std::cout << "connected\n";
  } else {
    std::cout << "Faled to connect err:\n " << ec.message() << '\n';
  }

  if (socket.is_open()) {
    // prime the ctx for reading before req
    grabSomeData(socket);
    // if connection was sucessfull
    std::string sReq = "GET /index.html HTTP/1.1\r\n"
                       "Host: example.com\r\n"
                       "Connection: close \r\n\r\n";

    // try to write to the server
    socket.write_some(asio::buffer(sReq.data(), sReq.size()), ec);
    /*
     MAUALY READING == BAD
      socket.wait(socket.wait_read);

      // try to get res

      size_t bytes = socket.available();
      std::cout << "bytes availible : " << bytes << '\n';

      // if bytes receved
      if (bytes > 0) {
        std::vector<char> vBuffer(bytes);
        socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);

        for (auto c : vBuffer) {
          std::cout << c;
        }
     }
     */

    // preventing premature close of program
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(20000ms);
  }

  return 0;
}
