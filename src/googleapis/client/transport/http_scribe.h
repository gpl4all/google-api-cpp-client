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

#ifndef APISERVING_CLIENTS_CPP_TRANSPORT_HTTP_SCRIBE_H_
#define APISERVING_CLIENTS_CPP_TRANSPORT_HTTP_SCRIBE_H_

#include <map>
using std::map;
#include <queue>
using std::queue;
#include <set>
using std::multiset;
using std::set;
#include <string>
using std::string;
#include "googleapis/base/integral_types.h"
#include "googleapis/base/macros.h"
#include "googleapis/base/mutex.h"
#include "googleapis/base/scoped_ptr.h"
#include "googleapis/base/thread_annotations.h"
#include "googleapis/strings/stringpiece.h"
#include "googleapis/util/status.h"
namespace googleapis {

namespace client {
class HttpRequest;
class ParsedUrl;

/*
 * Determines what is appropriate for scribes to record.
 * @ingroup TransportLayerCore
 *
 * The base class performs standard censoring
 *   - Authorization headers are scrubbed
 */
class HttpScribeCensor {
 public:
  /*
   * Standard constructor.
   */
  HttpScribeCensor();

  /*
   * Standard destructor.
   */
  virtual ~HttpScribeCensor();

  /*
   * Returns a scribbed url for the request.
   */
  virtual string GetCensoredUrl(
      const HttpRequest& request, bool* censored) const;

  /*
   * Returns a scrubbed request payload.
   *
   * The method should reset request->content_reader after reading
   * the contents.
   *
   * @param[in] request The request to censor.
   * @param[in] max_len The maximum length of body to keep.
   * @param[out] original_size The original uncensored size.
   * @param[out] censored Set to true if the result was censored.
   *
   * @return The censored request payload.
   */
  // TODO(ewiseblatt): 20130520
  // Really this should return a DataReader of either the original
  // reader or censored content but ownership cases are tricky and is
  // more vulnerable to rewind bugs so am leaving this as a string for now.
  virtual string GetCensoredRequestContent(
      const HttpRequest& request,
      int64 max_len,
      int64* original_size,
      bool* censored) const;

  /*
   * Returns a scrubbed responze payload.
   *
   * The method should reset response->body_reader after reading the contents.
   *
   * @param[in] request The request with the response to censor.
   * @param[in] max_len  The maximum length of body to keep.
   * @param[out] original_size The original uncensored size.
   * @param[out] censored Set to true if the result was censored.
   *
   * @return The censored response payload.
   */
  // TODO(ewiseblatt): 20130520
  // Really this should return a DataReader of either the original
  // reader or censored content but ownership cases are tricky and is
  // more vulnerable to rewind bugs so am leaving this as a string for now.
  virtual string GetCensoredResponseBody(
      const HttpRequest& request, int64 max_len,
      int64* original_size, bool* censored) const;

  /*
   * Returns a censored request header value.
   *
   * @param[in] request The request with the header.
   * @param[in] name The request header name.
   * @param[in] value The original value.
   * @param[out] censored True if the value was censored.
   *
   * @return The censored header value.
   */
  virtual string GetCensoredRequestHeaderValue(
      const HttpRequest& request,
      const StringPiece& name, const StringPiece& value,
      bool* censored) const;

  /*
   * Returns a censored response header value.
   *
   * @param[in] request The request with response having the header.
   * @param[in] name The response header name.
   * @param[in] value The original value.
   * @param[out] censored True if the value was censored.
   *
   * @return The censored header value.
   */
  virtual string GetCensoredResponseHeaderValue(
      const HttpRequest& request,
      const StringPiece& name, const StringPiece& value,
      bool* censored) const;

  /*
   * Returns the set of censored URL prefixes.
   *
   * These include the protocol (e.g. https://accounts.google.com)
   */
  const set<string>& censored_url_prefixes() const {
    return censored_url_prefixes_;
  }

  /*
   * Returns a mutable set of censored URL prefixes.
   *
   * These include the protocol (e.g. https://accounts.google.com).
   * You can add additional prefixes into the set.
   */
  set<string>* mutable_censored_url_prefixes() {
    return &censored_url_prefixes_;
  }

  /*
   * Returns a set of censored query parameter names.
   */
  const set<string>& censored_query_param_names() const {
    return censored_query_param_names_;
  }

  /*
   * Returns a mutable set of censored URL query parameters.
   *
   * You can add additional query parameter names into the set.
   */
  set<string>* mutable_censored_query_param_names() {
    return &censored_query_param_names_;
  }

  /*
   * Returns a set of censored HTTP request header names.
   */
  const set<string>& censored_request_header_names() const {
    return censored_request_header_names_;
  }

  /*
   * Returns a mutable set of censored HTTP request header names.
   *
   * You can add additional request header names into the set.
   */
  set<string>* mutable_censored_request_header_names() {
    return &censored_request_header_names_;
  }

  /*
   * Returns a set of censored HTTP response header names.
   */
  const set<string>& censored_response_header_names() const {
    return censored_response_header_names_;
  }

  /*
   * Returns a mutable set of censored HTTP reponse header names.
   *
   * You can add additional response header names into the set.
   */
  set<string>* mutable_censored_response_header_names() {
    return &censored_response_header_names_;
  }

 protected:
  /*
   * Censor the query part of a URL.
   *
   * @param[in] parsed_url The parsed url to extract from.
   * @param[out] censored Set to true if some values were censored.
   * @return censored query string.
   */
  string GetCensoredUrlQuery(
      const ParsedUrl& parsed_url, bool* censored) const;

 private:
  set<string> censored_url_prefixes_;
  set<string> censored_query_param_names_;
  set<string> censored_request_header_names_;
  set<string> censored_response_header_names_;

  bool IsSensitiveContent(const string& url) const;
  DISALLOW_COPY_AND_ASSIGN(HttpScribeCensor);
};

/*
 * Base class for scribe to log HTTP message activity.
 * @ingroup TransportLayerCore
 *
 * This is intended for debugging and diagnostics. A transport permits
 * binding a scribe to it to monitor transport activity. This class
 * provides the interface for the interaction between the HTTP requests
 * and scribes.
 *
 * Scribes should be thread-safe.
 *
 * @see HttpTransport::set_scribe
 */
class HttpScribe {
 public:
  /*
   * Standard destructor.
   */
  virtual ~HttpScribe();

  /*
   * Specifies the max size for captured content snippets.
   *
   * The default is unbounded so the whole message will be kept.
   * If you only need part of the payload then you can use this attribute
   * to restrict the size.
   *
   * @param[in] n most bytes to capture.
   */
  void set_max_snippet(int64 n)  { max_snippet_ = n; }

  /*
   * Returns the max size for captured snippets.
   *
   * @return bytes to capture.
   */
  int64 max_snippet() const  { return max_snippet_; }

  /*
   * Notification that the request is about to be sent to the transport.
   *
   * Since the scribe is used in the flow of normal sending activities,
   * it will be the caller's responsibility to reset the requset->body_reader.
   * after invoking this method.
   *
   * @param[in] request The request being sent.
   */
  virtual void AboutToSendRequest(const HttpRequest* request) = 0;

  /*
   * Notification that a request has received a response.
   *
   * Since the scribe is used in the flow of normal sending activities,
   * it will be the caller's responsibility to reset the
   * request->response->body_reader after invoking this method.
   *
   * @param[in] request The request holds its response.
   */
  virtual void ReceivedResponseForRequest(const HttpRequest* request) = 0;

  /*
   * Notification that a sent request has encountered a transport error.
   *
   * Transport errors are not HTTP errors. This is a timeout or network-level
   * error.
   *
   * @param[in] request The request that will never receive a response.
   * @param[in] error The transport error.
   */
  virtual void RequestFailedWithTransportError(
      const HttpRequest* request, const util::Status& error) = 0;

  void reset_censor(HttpScribeCensor* censor) {
    censor_.reset(censor);
  }

  const HttpScribeCensor* censor() { return censor_.get(); }

  /*
   * Checkpoint the scribe data into the writer if it
   * hasnt been done so already.
   */
  virtual void Checkpoint() = 0;

 protected:
  /*
   * Class is abstract so needs to be specialized to construct.
   *
   * @param[in] censor Provides policy for censoring data. Ownership is passed.
   */
  explicit HttpScribe(HttpScribeCensor* censor);

 private:
  scoped_ptr<HttpScribeCensor> censor_;
  int64 max_snippet_;
  DISALLOW_COPY_AND_ASSIGN(HttpScribe);
};

/*
 * A high level but still abstract class for intercepting HTTP requests.
 * @ingroup TransportLayerCore
 *
 * This class manages a collection of active HttpEntryScribe::Entry
 * instances with activity associated with them. This might be a more useful
 * starting point for viewing a collection of independent encapsulated
 * messages as opposed to a stream of interleaved events.
 */
class HttpEntryScribe : public HttpScribe {
 public:
  class Entry {
   public:
    /*
     * Standard constructor.
     *
     * @param[in] scribe The scribe constructing the entry.
     * @param[in] request The request that the entry is for.
     *
     * Caller should call either CancelAndDestroy or FlushAndDestroy when done.
     */
    Entry(HttpEntryScribe* scribe, const HttpRequest* request);

    /*
     * Finish recording the entry instance and destroy it.
     *
     * The entry has finished so should finish recording. Ownershp is passed
     * so that it can be destroyed. The entry is no longer in the scribe's
     * map or queue.
     *
     * This is called from the scribe within a critical section so does
     * not have to worry about threads.
     */
    virtual void FlushAndDestroy() = 0;

    /*
     * Destroys instance without flushing it so may forget about it.
     *
     * Intended to destory an instance without recording it.
     */
    virtual void CancelAndDestroy();

    /*
     * Hook for recording that the request was sent.
     *
     * @param[in] request The request is about to be executed.
     */
    virtual void Sent(const HttpRequest* request) = 0;

    /*
     * Hook for recording that the request received a response.
     *
     * @param[in] request The request has a response. The response
     * might be an HTTP failure.
     */
    virtual void Received(const HttpRequest* request) = 0;

    /*
     * Hook for recording that the request encountered a transport error.
     *
     * @param[in] request The request encountered a transport error so will
     *            never receive a response. It might have been previously sent
     *            but might not have.
     * @param[in] status Explains te error.
     */
    virtual void Failed(
        const HttpRequest* request, const util::Status& status) = 0;

    /*
     * Returns the age of this intance.
     *
     * @return microseconds since constructed.
     */
    int64 MicrosElapsed() const;

    /*
     * Returns bound scribe.
     *
     * @return scribe bound in constructor.
     */
    HttpEntryScribe* scribe() const { return scribe_; }

    /*
     * Returns bound request.
     *
     * @return request bound in constructor.
     */
    const HttpRequest* request() const  { return request_; }

    /*
     * Returns time instance was constructed.
     */
    const struct timeval& timeval() const  { return timeval_; }

    /*
     * Returns true if the request has completed.
     * If this is true then the request might have been destroyed already.
     *
     * The request will always be valid in the Sent, Received, and Failed
     * methods.
     *
     * @return true If the request is known to have finished.
     */
    bool done() const  { return done_; }

    /*
     * Informs the entry that the reqeust has finished.
     *
     * This method is called internally so you do not need to call this..
     */
    void set_done(bool is_done) { done_ = is_done; }

   protected:
    /*
     * Do not explicitly call this destructor.
     *
     * @see FlushAndDestroy
     * @see CancelAndDestroy
     */
    virtual ~Entry();  // Called by FlushAndDestroy or CancelAndDestroy

   private:
    HttpEntryScribe* scribe_;
    const HttpRequest* request_;
    struct timeval timeval_;
    bool done_;

    DISALLOW_COPY_AND_ASSIGN(Entry);
  };

  /*
   * Implements the HttpScribe::AboutToSendRequest method.
   *
   * This maps the request to an entry and calls the Entry::Sent method.
   *
   * @param[in] request The request that was sent.
   */
  virtual void AboutToSendRequest(const HttpRequest* request);

  /*
   * Implements the HttpScribe::ReceivedResponseForRequest method.
   *
   * This maps the request to an entry and calls the
   * Entry::Received method.
   *
   * @param[in] request The request whose response was received.
   */
  virtual void ReceivedResponseForRequest(const HttpRequest* request);

  /*
   * Implements the HttpScribe::RequestFailedWithTransportError method.
   *
   * This maps the request to an entry and calls the
   * Entry::Failed method.
   *
   * @param[in] request The request that had a transport failure.
   * @param[in] error An explanation of the failure.
   */
  virtual void RequestFailedWithTransportError(
      const HttpRequest* request, const util::Status& error);

 protected:
  /*
   * Standard constructor.
   *
   * @param[in] censor Ownership to the censorship policy is passed.
   */
  explicit HttpEntryScribe(HttpScribeCensor* censor);

  /*
   * Standard destructor.
   */
  virtual ~HttpEntryScribe();

  /*
   * Maps the request into a logical entry.
   *
   * @param[in] request The intercepted request.
   * @return the scribe maintains ownership for the entry consolidating
   *         activity for the request
   *
   * @see NewEntry
   * @see DiscardEntry
   */
  Entry* GetEntry(const HttpRequest* request) LOCKS_EXCLUDED(mutex_);

  /*
   * Unmaps and destroys the logical entry.
   *
   * This can be called to forget about the entry, but is normally handled
   * automatically by this scribe instance.
   *
   * @param[in,out] entry The entry will be destroyed.
   */
  void DiscardEntry(Entry* entry) LOCKS_EXCLUDED(mutex_);

  /*
   * Discard all the entries in the queue.
   *
   * Discarding the entries notifies the entry that it is done so it can
   * flush and remove itself. This method is here so that derived
   * destructors can call it if they need to finish the entry before
   * they destruct.
   *
   * The base class will call this to discard anything remaining in its
   * destructor.
   */
  void DiscardQueue();

  /*
   * Create a new entry instance for the request.
   *
   * This is called from within a critical section so does not have to
   * worry about threads.
   *
   * This lets specialized scribes return specialized entries that
   * drive the particular specialized behavior. It is called by
   * GetEntry.
   *
   * @return ownership of the entry.
   */
  virtual Entry* NewEntry(const HttpRequest* request)
      EXCLUSIVE_LOCKS_REQUIRED(mutex_)
      = 0;

  /*
   * Returns the entries that have not yet been unmapped.
   *
   * These are in the order they were created, which is probably
   * the order in which they were sent (minus race conditions).
   */
  queue<Entry*>* outstanding_queue() { return &queue_; }

 private:
  typedef map<const HttpRequest*, Entry*> EntryMap;
  typedef queue<Entry*> EntryQueue;

  void UnsafeDiscardEntry(Entry* entry) EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  Mutex mutex_;
  int64 sequence_number_ GUARDED_BY(mutex_);
  EntryMap map_ GUARDED_BY(mutex_);
  EntryQueue queue_ GUARDED_BY(mutex_);

  DISALLOW_COPY_AND_ASSIGN(HttpEntryScribe);
};

}  // namespace client

} // namespace googleapis
#endif  // APISERVING_CLIENTS_CPP_TRANSPORT_HTTP_SCRIBE_H_
