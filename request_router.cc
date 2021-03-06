#include "request_handlers.h"
#include "request_router.h"

RequestRouter::RequestRouter(ServerStats* server_stats) {
  server_stats_ = server_stats;
}

RequestRouter::~RequestRouter() {
  // TODO: do we need to delete the registered handlers?
  /*
  for (const auto pair : handlers_map_) {
    delete pair->second;
  }
  */
}

bool RequestRouter::buildRoutes(const ServerConfig& server_config) {
  // Iterate over uri_prefix -> handler_name map
  for (const auto path : server_config.allPaths()) {
    const std::string uri_prefix = path.first;
    const std::string handler_name = path.second;

    RequestHandler* handler_instance = RequestHandler::CreateByName(handler_name.c_str());
    if (handler_instance == nullptr) {
      return false;
    }
    else {
      if (! handler_instance->init(uri_prefix, *(server_config.getChildBlock(uri_prefix)))) {
        return false;
      }

      // If we have a StatusHandler, give it access to our ServerStats
      StatusHandler* status_handler = dynamic_cast<StatusHandler*>(handler_instance);
      if (status_handler != nullptr) {
        status_handler->setUpStats(server_stats_);
      }

      handlers_map_[uri_prefix] = std::move(handler_instance);
    }
  }
  std::cout << "Finished registering handlers\n";

  // Build special 404 route, which is our fall-through error case
  not_found_handler_ = RequestHandler::CreateByName("NotFoundRequestHandler");
  std::cout << "Finished creating not found handler\n";
  return true;
}

RequestHandler* RequestRouter::routeRequest(const std::string& full_uri) const {
  // Starting at the end of the string,
  // try longest-prefix match up to latest forward-slash
  //
  // Example: for the full_uri '/echo/subaction/subdir/file.txt',
  // we try '/echo/subaction/subdir', then '/echo/subaction',
  // and finally '/echo'.
  //
  // Note the lack of trailing slashes
  // See also: http://www.cplusplus.com/reference/string/string/find_first_of/
  for (int search_index = full_uri.size() - 1; search_index >= 0; search_index--) {
    size_t pos = full_uri.find_first_of('/', search_index);
    if (pos == std::string::npos) {
      continue;
    }
    else {
      // Slice string from 0 to pos; substr takes start and length
      // (unless there is only one '/', then use the whole uri)
      std::string possible_prefix;
      if (pos == 0) {
        possible_prefix = full_uri;
      }
      else {
        possible_prefix = full_uri.substr(0, pos);
      }

      std::cerr << "RequestRouter::routeRequest prefix substring = " << possible_prefix << std::endl;

      const auto it = handlers_map_.find(possible_prefix);
      if (it == handlers_map_.end()) {
        continue;
      }
      else {
        // Found the matching longest-prefix; return corresponding handler
        return it->second;
      }
    }
  }
  // If no prefix found, return the default handler
  // (a default handler must always be specified in the config)
  return handlers_map_.find("default")->second;
}

RequestHandler* RequestRouter::notFoundHandler() const {
  return not_found_handler_;
}
