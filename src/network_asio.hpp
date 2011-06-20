#ifndef NETWORK_ASIO_HPP_INCLUDED
#define NETWORK_ASIO_HPP_INCLUDED

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include "exceptions.hpp"
#include "config.hpp"

namespace network_asio {

struct error : public game::error
{
	error(const boost::system::error_code& error) : game::error(error.message()) {}
};

/** A class that represents a TCP/IP connection. */
class connection
{
	boost::asio::io_service io_service_;
	typedef boost::asio::ip::tcp::resolver resolver;
	resolver resolver_;

	typedef boost::asio::ip::tcp::socket socket;
	socket socket_;

	bool done_;

	boost::asio::streambuf write_buf_;
	boost::asio::streambuf read_buf_;

	void handle_resolve(
		const boost::system::error_code& ec,
		resolver::iterator iterator
		);
	
	void connect(resolver::iterator iterator);
	void handle_connect(
		const boost::system::error_code& ec,
		resolver::iterator iterator
		);
	void handshake();
	void handle_handshake(
		const boost::system::error_code& ec
		);
	union {
		char binary[4];
		boost::uint32_t num;
	} handshake_response_;

	void handle_write(
		const boost::system::error_code& ec,
		std::size_t bytes_transferred
		);
	std::size_t is_read_complete(
		const boost::system::error_code& error,
		std::size_t bytes_transferred
		); 
	void handle_read(
		const boost::system::error_code& ec,
		std::size_t bytes_transferred
		);
	boost::optional<std::size_t> bytes_to_read_;

	public:
	/**
	 * Constructor.
	 *
	 * @param host    Name of the host to connect to
	 * @param service Service identifier such as "80" or "http"
	 */
	connection(const std::string& host, const std::string& service);

	void transfer(const config& request, config& response);

	/** Handle all pending asynchonous events and return */
	std::size_t poll()
	{
		try {
			return io_service_.poll();
		} catch(const boost::system::system_error& err) {
			throw error(err.code());
		}
	}
	/** Run asio's event loop
	 *
	 * Handle asynchronous events blocking until all asynchronous
	 * operations have finished
	 */
	void run() { io_service_.run(); }

	/** True if connected and no high-level operation is in progress */
	bool done() const { return done_; }
};

}

#endif
