/*
    Copyright 2015, Proteus Tech Co Ltd. http://www.kirit.com/Rask
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/internet>
#include <fost/http.server.hpp>
#include <fost/log>
#include <fost/main>

#include <rask/pool.hpp>


namespace {


    const fostlib::setting<fostlib::json> c_logger(
        "rask/main.cpp", "rask", "logging", fostlib::null, true);

    bool webserver(fostlib::http::server::request &req) {
        fostlib::text_body response(
            fostlib::utf8_string("<html><body>Rask web server</body></html>"),
            fostlib::mime::mime_headers(), "text/html");
        req( response );
        return true;
    }


}


FSL_MAIN("rask", "Rask")(fostlib::ostream &out, fostlib::arguments &args) {
    // Load the configuration files we've been given on the command line
    std::vector<fostlib::settings> configuration;
    configuration.reserve(args.size());
    for ( std::size_t arg{1}; arg != args.size(); ++arg ) {
        auto filename = fostlib::coerce<boost::filesystem::path>(args[arg].value());
        out << "Loading config " << filename << std::endl;
        configuration.emplace_back(filename);
    }
    // Set up the logging options
    std::unique_ptr<fostlib::log::global_sink_configuration> loggers;
    if ( !c_logger.value().isnull() && c_logger.value().has_key("sinks") ) {
        loggers = std::make_unique<fostlib::log::global_sink_configuration>(c_logger.value());
    }
    // Start the threads for doing work
    rask::pool io(4), hashers(2);
    fostlib::log::debug("Started Rask");
    // Spin up the web server
    fostlib::http::server server(fostlib::host(), 4000);
    server(webserver); // This will never return
    return 0;
}
