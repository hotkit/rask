/*
    Copyright 2015, Proteus Tech Co Ltd. http://www.kirit.com/Rask
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <beanbag/beanbag>
#include <fost/internet>
#include <fost/http.server.hpp>
#include <fost/log>
#include <fost/main>
#include <fost/urlhandler>

#include <rask/tenants.hpp>
#include <rask/pool.hpp>


namespace {


    const fostlib::setting<fostlib::json> c_logger(
        "rask/main.cpp", "rask", "logging", fostlib::json(), true);
    const fostlib::setting<fostlib::json> c_server_db(
        "rask/main.cpp", "rask", "server", fostlib::json(), true);
    const fostlib::setting<fostlib::json> c_tenant_db(
        "rask/main.cpp", "rask", "tenants", fostlib::json(), true);


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
    // Work out server identity
    if ( !c_server_db.value().isnull() ) {
        beanbag::jsondb_ptr dbp(beanbag::database(c_server_db.value()));
        fostlib::jsondb::local server(*dbp);
        if ( !server.has_key("identity") ) {
            uint32_t random = 0;
            std::ifstream urandom("/dev/urandom");
            random += urandom.get() << 16;
            random += urandom.get() << 8;
            random += urandom.get();
            random &= (1 << 20) - 1; // Take 20 bits
            server.set("identity", random);
            server.commit();
            fostlib::log::info()("Server identity picked as", random);
        }
    }
    // Start the threads for doing work
    rask::pool io(4), hashers(2);
    // TODO: Start listening for connections
    // Load tenants
    if ( !c_tenant_db.value().isnull() ) {
        rask::tenants(c_tenant_db.value());
    }
    // TODO: Connect to peers
    // Log that we've started
    fostlib::log::debug("Started Rask, spinning up web server");
    // Spin up the web server
    fostlib::http::server server(fostlib::host(), 4000);
    server(fostlib::urlhandler::service); // This will never return
    return 0;
}
