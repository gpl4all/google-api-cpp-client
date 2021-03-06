/*
 * \copyright Copyright 2013 Google Inc. All Rights Reserved.
 * \license @{
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @}
 */


#ifndef APISERVING_CLIENTS_CPP_UTIL_MONGOOSE_WEBSERVER_H_
#define APISERVING_CLIENTS_CPP_UTIL_MONGOOSE_WEBSERVER_H_

#include <map>
using std::map;
#include <vector>
using std::vector;
#include "googleapis/client/util/abstract_webserver.h"
#include "googleapis/base/callback.h"
#include "googleapis/base/macros.h"
#include "googleapis/strings/stringpiece.h"
#include "googleapis/util/status.h"
#include <mongoose/mongoose.h>
namespace googleapis {

namespace client {

/*
 * Provides a web server for samples and testing.
 * @ingroup PlatformLayerWebServer
 *
 * This class and the use of Mongoose Web Server available from
 * https://code.google.com/p/mongoose/
 *
 * This class is currently only intended to support testing and tinkering.
 * It is not robust for production use. The underlying library is probably
 * sufficient, but this wrapper uses only minimal configuration and processing.
 */
class MongooseWebServer : public AbstractWebServer {
 public:
  static const StringPiece ACCESS_LOG_FILE;
  static const StringPiece DOCUMENT_ROOT;
  static const StringPiece ENABLE_KEEP_ALIVE;
  static const StringPiece ERROR_LOG_FILE;
  static const StringPiece LISTENING_PORTS;
  static const StringPiece NUM_THREADS;
  static const StringPiece REQUEST_TIMEOUT_MS;
  static const StringPiece SSL_CERTIFICATE;

  /*
   * Constructs an http server on the given port
   *
   * @param[in] port Should be non-0.
   */
  explicit MongooseWebServer(int port);

  /*
   * Standard destructor.
   */
  virtual ~MongooseWebServer();

  /*
   * Determines whether we are using SSL or not.
   * @return if we'll use SSL (https).
   */
  bool use_ssl() const {
    return !mongoose_option(kSslCertificateOption).empty();
  }

  /*
   * Override Mongoose options.
   *
   * This replaces the old options that were overriden.
   *
   * @param[in] options The options will be copied. They must be set
   *            before the server is started.
   */
  void set_mongoose_options(const map<string, string>& options) {
    options_ = options;
  }

  /*
   * Returns Mongoose options that were overriden.
   */
  const map<string, string>& mongoose_options() const { return options_; }

  /*
   * Clears Mongoose option overrides.
   */
  void clear_mongoose_options() { options_.clear(); }

  /*
   * Explicitly configure an individual Mongoose server option.
   *
   * @param[in] name See the Mongoose Documentation for option names.
   * @param[in] value See the Mongoose Documentation for option values.
   */
  void set_mongoose_option(
      const StringPiece& name, const StringPiece& value) {
    options_.insert(make_pair(name.as_string(), value.as_string()));
  }

  /*
   * Returns value for individual option, or empty if not set.
   */
  const string mongoose_option(const string& name) const {
    map<string, string>::const_iterator it = options_.find(name);
    if (it == options_.end()) return "";
    return it->second;
  }

  /*
   * Clears an overiden Mongoose options back to the default value.
   */
  void clear_mongoose_option(const StringPiece& name) {
    map<string, string>::iterator it = options_.find(name.as_string());
    if (it != options_.end()) {
      options_.erase(it);
    }
  }

  /*
   * Returns actual protocol depending on whether SSL was enabled.
   */
  virtual string url_protocol() const;

 protected:
  /*
   * Starts the server.
   *
   * @return ok or reason for error.
   */
  virtual util::Status DoStartup();

  /*
   * Stops the server.
   *
   * @return ok or reason for error.
   */
  virtual void DoShutdown();

 protected:
  /*
   * Sends the body response with the given http_code.
   *
   * @param[in] content_type The MIME content_type for the response.
   * @param[in] http_code The HTTP status code to return.
   * @param[in] body The HTTP body to return.
   * @param[in] connection The connection passed to the DoHandleUrl.
   */
  int SendResponse(
      const StringPiece& content_type,
      int http_code,
      const StringPiece& body,
      struct mg_connection* connection);

 private:
  const string kSslCertificateOption;
  std::map<string, string> options_;
  struct mg_callbacks callbacks_;
  struct mg_context* mg_context_;

  static int BeginRequestHandler(struct mg_connection* conn);

  DISALLOW_COPY_AND_ASSIGN(MongooseWebServer);
};

}  // namespace client

} // namespace googleapis
#endif  // APISERVING_CLIENTS_CPP_UTIL_MONGOOSE_WEBSERVER_H_
