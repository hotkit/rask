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
#include <fost/urlhandler>

#include <rask/configuration.hpp>
#include <rask/peer.hpp>
#include <rask/server.hpp>
#include <rask/tenants.hpp>
#include <rask/workers.hpp>


namespace {


    const fostlib::setting<fostlib::json> c_logger(
        "rask/main.cpp", "rask", "logging", fostlib::json(), true);
    // Take out the Fost logger configuration so we don't end up with both
    const fostlib::setting<fostlib::json> c_fost_logger(
        "rask/main.cpp", "rask", "Logging sinks", fostlib::json::parse(
            "{\"sinks\":[]}"));
    const fostlib::setting<uint16_t> c_webserver_port(
        "rask/main.cpp", "rask", "webserver-port", 4000, true);


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
    // TODO: Handle extra switches
    // Set up the logging options
    std::unique_ptr<fostlib::log::global_sink_configuration> loggers;
    if ( !c_logger.value().isnull() && c_logger.value().has_key("sinks") ) {
        loggers = std::make_unique<fostlib::log::global_sink_configuration>(c_logger.value());
    }
    // Start the threads for doing work
    rask::workers workers;
    // Spin up the Rask server
    rask::server(workers);
    // Load tenants and start sweeping
    if ( !rask::c_tenant_db.value().isnull() ) {
        rask::tenants(workers, rask::c_tenant_db.value());
        workers.notify();
    }
    // Connect to peers
    if ( !rask::c_peers_db.value().isnull() ) {
        rask::peer(workers, rask::c_peers_db.value());
    }
    // All done, finally start the web server (or whatever)
    if ( c_webserver_port.value() ) {
        // Log that we've started
        fostlib::log::info("Started Rask, spinning up web server");
        // Spin up the web server
        fostlib::http::server server(fostlib::host(), c_webserver_port.value());
        server(fostlib::urlhandler::service); // This will never return
    } else {
        // Log that we're sleeping
        fostlib::log::warning("Started Rask without a webserver -- press RETURN to exit");
        std::cin.get();
    }
    return 0;
}
