/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#pragma once

#if ((__GNUC__ >= 4) || defined(__clang__)) && defined(S2N_EXPORTS)
#    define S2N_API __attribute__((visibility("default")))
#else
#    define S2N_API
#endif /* __GNUC__ >= 4 || defined(__clang__) */

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/uio.h>

/* Function return code  */
#define S2N_SUCCESS 0
#define S2N_FAILURE -1

/* Callback return code */
#define S2N_CALLBACK_BLOCKED -2

#define S2N_MINIMUM_SUPPORTED_TLS_RECORD_MAJOR_VERSION 2
#define S2N_MAXIMUM_SUPPORTED_TLS_RECORD_MAJOR_VERSION 3
#define S2N_SSLv2 20
#define S2N_SSLv3 30
#define S2N_TLS10 31
#define S2N_TLS11 32
#define S2N_TLS12 33
#define S2N_TLS13 34
#define S2N_UNKNOWN_PROTOCOL_VERSION 0

S2N_API
extern __thread int s2n_errno;

/**
 * Returns the address of the thread-local `s2n_errno` variable
 *
 * This function can be used instead of trying to resolve `s2n_errno` directly
 * in runtimes where thread-local variables may not be easily accessible.
 */
S2N_API
extern int *s2n_errno_location(void);

typedef enum {
    S2N_ERR_T_OK=0,
    S2N_ERR_T_IO,
    S2N_ERR_T_CLOSED,
    S2N_ERR_T_BLOCKED,
    S2N_ERR_T_ALERT,
    S2N_ERR_T_PROTO,
    S2N_ERR_T_INTERNAL,
    S2N_ERR_T_USAGE
} s2n_error_type;

S2N_API
extern int s2n_error_get_type(int error);

struct s2n_config;
struct s2n_connection;

S2N_API
extern unsigned long s2n_get_openssl_version(void);
S2N_API
extern int s2n_init(void);
S2N_API
extern int s2n_cleanup(void);
S2N_API
extern struct s2n_config *s2n_config_new(void);
S2N_API
extern int s2n_config_free(struct s2n_config *config);
S2N_API
extern int s2n_config_free_dhparams(struct s2n_config *config);
S2N_API
extern int s2n_config_free_cert_chain_and_key(struct s2n_config *config);

typedef int (*s2n_clock_time_nanoseconds) (void *, uint64_t *);
typedef int (*s2n_cache_retrieve_callback) (struct s2n_connection *conn, void *, const void *key, uint64_t key_size, void *value, uint64_t *value_size);
typedef int (*s2n_cache_store_callback) (struct s2n_connection *conn, void *, uint64_t ttl_in_seconds, const void *key, uint64_t key_size, const void *value, uint64_t value_size);
typedef int (*s2n_cache_delete_callback) (struct s2n_connection *conn,  void *, const void *key, uint64_t key_size);

S2N_API
extern int s2n_config_set_wall_clock(struct s2n_config *config, s2n_clock_time_nanoseconds clock_fn, void *ctx);
S2N_API
extern int s2n_config_set_monotonic_clock(struct s2n_config *config, s2n_clock_time_nanoseconds clock_fn, void *ctx);

S2N_API
extern const char *s2n_strerror(int error, const char *lang);
S2N_API
extern const char *s2n_strerror_debug(int error, const char *lang);
S2N_API
extern const char *s2n_strerror_name(int error); 

struct s2n_stacktrace;
S2N_API
extern bool s2n_stack_traces_enabled(void);
S2N_API
extern int s2n_stack_traces_enabled_set(bool newval);
S2N_API
extern int s2n_calculate_stacktrace(void);
S2N_API
extern int s2n_print_stacktrace(FILE *fptr);
S2N_API
extern int s2n_free_stacktrace(void);
S2N_API
extern int s2n_get_stacktrace(struct s2n_stacktrace *trace);

S2N_API
extern int s2n_config_set_cache_store_callback(struct s2n_config *config, s2n_cache_store_callback cache_store_callback, void *data);
S2N_API
extern int s2n_config_set_cache_retrieve_callback(struct s2n_config *config, s2n_cache_retrieve_callback cache_retrieve_callback, void *data);
S2N_API
extern int s2n_config_set_cache_delete_callback(struct s2n_config *config, s2n_cache_delete_callback cache_delete_callback, void *data);

typedef int (*s2n_mem_init_callback)(void);
typedef int (*s2n_mem_cleanup_callback)(void);
typedef int (*s2n_mem_malloc_callback)(void **ptr, uint32_t requested, uint32_t *allocated);
typedef int (*s2n_mem_free_callback)(void *ptr, uint32_t size);

S2N_API
extern int s2n_mem_set_callbacks(s2n_mem_init_callback mem_init_callback, s2n_mem_cleanup_callback mem_cleanup_callback,
                                 s2n_mem_malloc_callback mem_malloc_callback, s2n_mem_free_callback mem_free_callback);

typedef int (*s2n_rand_init_callback)(void);
typedef int (*s2n_rand_cleanup_callback)(void);
typedef int (*s2n_rand_seed_callback)(void *data, uint32_t size);
typedef int (*s2n_rand_mix_callback)(void *data, uint32_t size);

S2N_API
extern int s2n_rand_set_callbacks(s2n_rand_init_callback rand_init_callback, s2n_rand_cleanup_callback rand_cleanup_callback,
        s2n_rand_seed_callback rand_seed_callback, s2n_rand_mix_callback rand_mix_callback);

typedef enum {
    S2N_EXTENSION_SERVER_NAME = 0,
    S2N_EXTENSION_MAX_FRAG_LEN = 1,
    S2N_EXTENSION_OCSP_STAPLING = 5,
    S2N_EXTENSION_SUPPORTED_GROUPS = 10,
    S2N_EXTENSION_EC_POINT_FORMATS = 11,
    S2N_EXTENSION_SIGNATURE_ALGORITHMS = 13,
    S2N_EXTENSION_ALPN = 16,
    S2N_EXTENSION_CERTIFICATE_TRANSPARENCY = 18,
    S2N_EXTENSION_RENEGOTIATION_INFO = 65281,
} s2n_tls_extension_type;

typedef enum {
    S2N_TLS_MAX_FRAG_LEN_512 = 1,
    S2N_TLS_MAX_FRAG_LEN_1024 = 2,
    S2N_TLS_MAX_FRAG_LEN_2048 = 3,
    S2N_TLS_MAX_FRAG_LEN_4096 = 4,
} s2n_max_frag_len;

struct s2n_cert;
struct s2n_cert_chain_and_key;
struct s2n_pkey;
typedef struct s2n_pkey s2n_cert_public_key;
typedef struct s2n_pkey s2n_cert_private_key;

S2N_API
extern struct s2n_cert_chain_and_key *s2n_cert_chain_and_key_new(void);
S2N_API
extern int s2n_cert_chain_and_key_load_pem(struct s2n_cert_chain_and_key *chain_and_key, const char *chain_pem, const char *private_key_pem);
S2N_API
extern int s2n_cert_chain_and_key_load_pem_bytes(struct s2n_cert_chain_and_key *chain_and_key, uint8_t *chain_pem, uint32_t chain_pem_len, uint8_t *private_key_pem, uint32_t private_key_pem_len);
S2N_API
extern int s2n_cert_chain_and_key_free(struct s2n_cert_chain_and_key *cert_and_key);
S2N_API
extern int s2n_cert_chain_and_key_set_ctx(struct s2n_cert_chain_and_key *cert_and_key, void *ctx);
S2N_API
extern void *s2n_cert_chain_and_key_get_ctx(struct s2n_cert_chain_and_key *cert_and_key);
S2N_API
extern s2n_cert_private_key *s2n_cert_chain_and_key_get_private_key(struct s2n_cert_chain_and_key *cert_and_key);

typedef struct s2n_cert_chain_and_key* (*s2n_cert_tiebreak_callback) (struct s2n_cert_chain_and_key *cert1, struct s2n_cert_chain_and_key *cert2, uint8_t *name, uint32_t name_len);
S2N_API
extern int s2n_config_set_cert_tiebreak_callback(struct s2n_config *config, s2n_cert_tiebreak_callback cert_tiebreak_cb);

S2N_API
extern int s2n_config_add_cert_chain_and_key(struct s2n_config *config, const char *cert_chain_pem, const char *private_key_pem);
S2N_API
extern int s2n_config_add_cert_chain_and_key_to_store(struct s2n_config *config, struct s2n_cert_chain_and_key *cert_key_pair);
S2N_API
extern int s2n_config_set_cert_chain_and_key_defaults(struct s2n_config *config,
                                                      struct s2n_cert_chain_and_key **cert_key_pairs,
                                                      uint32_t num_cert_key_pairs);

S2N_API
extern int s2n_config_set_verification_ca_location(struct s2n_config *config, const char *ca_pem_filename, const char *ca_dir);
S2N_API
extern int s2n_config_add_pem_to_trust_store(struct s2n_config *config, const char *pem);

typedef uint8_t (*s2n_verify_host_fn) (const char *host_name, size_t host_name_len, void *data);
/* will be inherited by s2n_connection. If s2n_connection specifies a callback, that callback will be used for that connection. */
S2N_API
extern int s2n_config_set_verify_host_callback(struct s2n_config *config, s2n_verify_host_fn, void *data);

S2N_API
extern int s2n_config_set_check_stapled_ocsp_response(struct s2n_config *config, uint8_t check_ocsp);
S2N_API
extern int s2n_config_disable_x509_verification(struct s2n_config *config);
S2N_API
extern int s2n_config_set_max_cert_chain_depth(struct s2n_config *config, uint16_t max_depth);

S2N_API
extern int s2n_config_add_dhparams(struct s2n_config *config, const char *dhparams_pem);
S2N_API
extern int s2n_config_set_cipher_preferences(struct s2n_config *config, const char *version);

/**
 * Appends the provided application protocol to the preference list
 *
 * The data provided in `protocol` parameter will be copied into an internal buffer
 *
 * @param config The configuration object being updated
 * @param protocol A pointer to a byte array value
 * @param protocol_len The length of bytes that should be read from `protocol`. Note: this value cannot be 0, otherwise an error will be returned.
 */
S2N_API
extern int s2n_config_append_protocol_preference(struct s2n_config *config, const uint8_t *protocol, uint8_t protocol_len);

S2N_API
extern int s2n_config_set_protocol_preferences(struct s2n_config *config, const char * const *protocols, int protocol_count);
typedef enum { S2N_STATUS_REQUEST_NONE = 0, S2N_STATUS_REQUEST_OCSP = 1 } s2n_status_request_type;
S2N_API
extern int s2n_config_set_status_request_type(struct s2n_config *config, s2n_status_request_type type);
typedef enum { S2N_CT_SUPPORT_NONE = 0, S2N_CT_SUPPORT_REQUEST = 1 } s2n_ct_support_level;
S2N_API
extern int s2n_config_set_ct_support_level(struct s2n_config *config, s2n_ct_support_level level);
typedef enum { S2N_ALERT_FAIL_ON_WARNINGS = 0, S2N_ALERT_IGNORE_WARNINGS = 1 } s2n_alert_behavior;
S2N_API
extern int s2n_config_set_alert_behavior(struct s2n_config *config, s2n_alert_behavior alert_behavior);
S2N_API
extern int s2n_config_set_extension_data(struct s2n_config *config, s2n_tls_extension_type type, const uint8_t *data, uint32_t length);
S2N_API
extern int s2n_config_send_max_fragment_length(struct s2n_config *config, s2n_max_frag_len mfl_code);
S2N_API
extern int s2n_config_accept_max_fragment_length(struct s2n_config *config);

S2N_API
extern int s2n_config_set_session_state_lifetime(struct s2n_config *config, uint64_t lifetime_in_secs);

S2N_API
extern int s2n_config_set_session_tickets_onoff(struct s2n_config *config, uint8_t enabled);
S2N_API
extern int s2n_config_set_session_cache_onoff(struct s2n_config *config, uint8_t enabled);
S2N_API
extern int s2n_config_set_ticket_encrypt_decrypt_key_lifetime(struct s2n_config *config, uint64_t lifetime_in_secs);
S2N_API
extern int s2n_config_set_ticket_decrypt_key_lifetime(struct s2n_config *config, uint64_t lifetime_in_secs);
S2N_API
extern int s2n_config_add_ticket_crypto_key(struct s2n_config *config,
                                            const uint8_t *name, uint32_t name_len,
                                            uint8_t *key, uint32_t key_len,
                                            uint64_t intro_time_in_seconds_from_epoch);

typedef enum { S2N_SERVER, S2N_CLIENT } s2n_mode;
S2N_API
extern struct s2n_connection *s2n_connection_new(s2n_mode mode);
S2N_API
extern int s2n_connection_set_config(struct s2n_connection *conn, struct s2n_config *config);

S2N_API
extern int s2n_connection_set_ctx(struct s2n_connection *conn, void *ctx);
S2N_API
extern void *s2n_connection_get_ctx(struct s2n_connection *conn);

typedef int s2n_client_hello_fn(struct s2n_connection *conn, void *ctx);
typedef enum { S2N_CLIENT_HELLO_CB_BLOCKING, S2N_CLIENT_HELLO_CB_NONBLOCKING } s2n_client_hello_cb_mode;
S2N_API
extern int s2n_config_set_client_hello_cb(struct s2n_config *config, s2n_client_hello_fn client_hello_callback, void *ctx);
S2N_API
extern int s2n_config_set_client_hello_cb_mode(struct s2n_config *config, s2n_client_hello_cb_mode cb_mode);
S2N_API
extern int s2n_client_hello_cb_done(struct s2n_connection *conn);
S2N_API
extern int s2n_connection_server_name_extension_used(struct s2n_connection *conn);

struct s2n_client_hello;
S2N_API
extern struct s2n_client_hello *s2n_connection_get_client_hello(struct s2n_connection *conn);
S2N_API
extern ssize_t s2n_client_hello_get_raw_message_length(struct s2n_client_hello *ch);
S2N_API
extern ssize_t s2n_client_hello_get_raw_message(struct s2n_client_hello *ch, uint8_t *out, uint32_t max_length);
S2N_API
extern ssize_t s2n_client_hello_get_cipher_suites_length(struct s2n_client_hello *ch);
S2N_API
extern ssize_t s2n_client_hello_get_cipher_suites(struct s2n_client_hello *ch, uint8_t *out, uint32_t max_length);
S2N_API
extern ssize_t s2n_client_hello_get_extensions_length(struct s2n_client_hello *ch);
S2N_API
extern ssize_t s2n_client_hello_get_extensions(struct s2n_client_hello *ch, uint8_t *out, uint32_t max_length);
S2N_API
extern ssize_t s2n_client_hello_get_extension_length(struct s2n_client_hello *ch, s2n_tls_extension_type extension_type);
S2N_API
extern ssize_t s2n_client_hello_get_extension_by_id(struct s2n_client_hello *ch, s2n_tls_extension_type extension_type, uint8_t *out, uint32_t max_length);

S2N_API
extern int s2n_connection_set_fd(struct s2n_connection *conn, int fd);
S2N_API
extern int s2n_connection_set_read_fd(struct s2n_connection *conn, int readfd);
S2N_API
extern int s2n_connection_set_write_fd(struct s2n_connection *conn, int writefd);
S2N_API
extern int s2n_connection_use_corked_io(struct s2n_connection *conn);

typedef int s2n_recv_fn(void *io_context, uint8_t *buf, uint32_t len);
typedef int s2n_send_fn(void *io_context, const uint8_t *buf, uint32_t len);
S2N_API
extern int s2n_connection_set_recv_ctx(struct s2n_connection *conn, void *ctx);
S2N_API
extern int s2n_connection_set_send_ctx(struct s2n_connection *conn, void *ctx);
S2N_API
extern int s2n_connection_set_recv_cb(struct s2n_connection *conn, s2n_recv_fn recv);
S2N_API
extern int s2n_connection_set_send_cb(struct s2n_connection *conn, s2n_send_fn send);

S2N_API
extern int s2n_connection_prefer_throughput(struct s2n_connection *conn);
S2N_API
extern int s2n_connection_prefer_low_latency(struct s2n_connection *conn);
S2N_API
extern int s2n_connection_set_dynamic_record_threshold(struct s2n_connection *conn, uint32_t resize_threshold, uint16_t timeout_threshold);

/* If you don't want to use the configuration wide callback, you can set this per connection and it will be honored. */
S2N_API
extern int s2n_connection_set_verify_host_callback(struct s2n_connection *config, s2n_verify_host_fn host_fn, void *data);

typedef enum { S2N_BUILT_IN_BLINDING, S2N_SELF_SERVICE_BLINDING } s2n_blinding;
S2N_API
extern int s2n_connection_set_blinding(struct s2n_connection *conn, s2n_blinding blinding);
S2N_API
extern uint64_t s2n_connection_get_delay(struct s2n_connection *conn);

S2N_API
extern int s2n_connection_set_cipher_preferences(struct s2n_connection *conn, const char *version);

/**
 * Appends the provided application protocol to the preference list
 *
 * The data provided in `protocol` parameter will be copied into an internal buffer
 *
 * @param conn The connection object being updated
 * @param protocol A pointer to a slice of bytes
 * @param protocol_len The length of bytes that should be read from `protocol`. Note: this value cannot be 0, otherwise an error will be returned.
 */
S2N_API
extern int s2n_connection_append_protocol_preference(struct s2n_connection *conn, const uint8_t *protocol, uint8_t protocol_len);

S2N_API
extern int s2n_connection_set_protocol_preferences(struct s2n_connection *conn, const char * const *protocols, int protocol_count);
S2N_API
extern int s2n_set_server_name(struct s2n_connection *conn, const char *server_name);
S2N_API
extern const char *s2n_get_server_name(struct s2n_connection *conn);
S2N_API
extern const char *s2n_get_application_protocol(struct s2n_connection *conn);
S2N_API
extern const uint8_t *s2n_connection_get_ocsp_response(struct s2n_connection *conn, uint32_t *length);
S2N_API
extern const uint8_t *s2n_connection_get_sct_list(struct s2n_connection *conn, uint32_t *length);

typedef enum {
    S2N_NOT_BLOCKED = 0,
    S2N_BLOCKED_ON_READ,
    S2N_BLOCKED_ON_WRITE,
    S2N_BLOCKED_ON_APPLICATION_INPUT,
    S2N_BLOCKED_ON_EARLY_DATA,
} s2n_blocked_status;
S2N_API
extern int s2n_negotiate(struct s2n_connection *conn, s2n_blocked_status *blocked);
S2N_API
extern ssize_t s2n_send(struct s2n_connection *conn, const void *buf, ssize_t size, s2n_blocked_status *blocked);
S2N_API
extern ssize_t s2n_sendv(struct s2n_connection *conn, const struct iovec *bufs, ssize_t count, s2n_blocked_status *blocked);
S2N_API
extern ssize_t s2n_sendv_with_offset(struct s2n_connection *conn, const struct iovec *bufs, ssize_t count, ssize_t offs, s2n_blocked_status *blocked);
S2N_API
extern ssize_t s2n_recv(struct s2n_connection *conn,  void *buf, ssize_t size, s2n_blocked_status *blocked);
S2N_API
extern uint32_t s2n_peek(struct s2n_connection *conn);

S2N_API
extern int s2n_connection_free_handshake(struct s2n_connection *conn);
S2N_API
extern int s2n_connection_release_buffers(struct s2n_connection *conn);
S2N_API
extern int s2n_connection_wipe(struct s2n_connection *conn);
S2N_API
extern int s2n_connection_free(struct s2n_connection *conn);
S2N_API
extern int s2n_shutdown(struct s2n_connection *conn, s2n_blocked_status *blocked);

typedef enum { S2N_CERT_AUTH_NONE, S2N_CERT_AUTH_REQUIRED, S2N_CERT_AUTH_OPTIONAL } s2n_cert_auth_type;

S2N_API
extern int s2n_config_get_client_auth_type(struct s2n_config *config, s2n_cert_auth_type *client_auth_type);
S2N_API
extern int s2n_config_set_client_auth_type(struct s2n_config *config, s2n_cert_auth_type client_auth_type);
S2N_API
extern int s2n_connection_get_client_auth_type(struct s2n_connection *conn, s2n_cert_auth_type *client_auth_type);
S2N_API
extern int s2n_connection_set_client_auth_type(struct s2n_connection *conn, s2n_cert_auth_type client_auth_type);
S2N_API
extern int s2n_connection_get_client_cert_chain(struct s2n_connection *conn, uint8_t **der_cert_chain_out, uint32_t *cert_chain_len);

/**
 * Sets the initial number of session tickets to send after a >=TLS1.3 handshake. The default value is one ticket.
 *
 * @param config A pointer to the config object.
 * @param num The number of session tickets that will be sent.
 */
S2N_API
extern int s2n_config_set_initial_ticket_count(struct s2n_config *config, uint8_t num);

/**
 * Increases the number of session tickets to send after a >=TLS1.3 handshake.
 *
 * @param conn A pointer to the connection object.
 * @param num The number of additional session tickets to send.
 */
S2N_API
extern int s2n_connection_add_new_tickets_to_send(struct s2n_connection *conn, uint8_t num);

/**
 * Sets the keying material lifetime for >=TLS1.3 session tickets so that one session doesn't get re-used ad infinitum.
 * The default value is one week.
 *
 * @param conn A pointer to the connection object.
 * @param lifetime_in_secs Lifetime of keying material in seconds.
 */
S2N_API
extern int s2n_connection_set_server_keying_material_lifetime(struct s2n_connection *conn, uint32_t lifetime_in_secs);

struct s2n_session_ticket;

/**
 * Callback function for receiving a session ticket.
 *
 * # Safety
 *
 * `ctx` is a void pointer and the caller is responsible for ensuring it is cast to the correct type.
 * `ticket` is valid only within the scope of this callback.
 *
 * @param conn A pointer to the connection object.
 * @param ctx Context for the session ticket callback function.
 * @param ticket Pointer to the received session ticket object.
 */
typedef int (*s2n_session_ticket_fn)(struct s2n_connection *conn, void *ctx, struct s2n_session_ticket *ticket);

/**
 * Sets a session ticket callback to be called when a client receives a new session ticket.
 *
 * # Safety
 *
 * `callback` MUST cast `ctx` into the same type of pointer that was originally created.
 * `ctx` MUST be valid for the lifetime of the config, or until a different context is set.
 *
 * @param config A pointer to the config object.
 * @param callback The function that should be called when the callback is triggered.
 * @param ctx The context to be passed when the callback is called.
 */
S2N_API
extern int s2n_config_set_session_ticket_cb(struct s2n_config *config, s2n_session_ticket_fn callback, void *ctx);

/**
 * Gets the length of the session ticket from a session ticket object.
 *
 * @param ticket Pointer to the session ticket object.
 * @param data_len Pointer to be set to the length of the session ticket on success.
 */
S2N_API
extern int s2n_session_ticket_get_data_len(struct s2n_session_ticket *ticket, size_t *data_len);

/**
 * Gets the session ticket data from a session ticket object.
 *
 * # Safety
 * The entire session ticket will be copied into `data` on success. Therefore, `data` MUST have enough
 * memory to store the session ticket data.
 *
 * @param ticket Pointer to the session ticket object.
 * @param max_data_len Maximum length of data that can be written to the 'data' pointer.
 * @param data Pointer to where the session ticket data will be stored.
 */
S2N_API
extern int s2n_session_ticket_get_data(struct s2n_session_ticket *ticket, size_t max_data_len, uint8_t *data);

/**
 * Gets the lifetime in seconds of the session ticket from a session ticket object.
 *
 * @param ticket Pointer to the session ticket object.
 * @param session_lifetime Pointer to a variable where the lifetime of the session ticket will be stored.
 */
S2N_API
extern int s2n_session_ticket_get_lifetime(struct s2n_session_ticket *ticket, uint32_t *session_lifetime);

S2N_API
extern int s2n_connection_set_session(struct s2n_connection *conn, const uint8_t *session, size_t length);
S2N_API
extern int s2n_connection_get_session(struct s2n_connection *conn, uint8_t *session, size_t max_length);
S2N_API
extern int s2n_connection_get_session_ticket_lifetime_hint(struct s2n_connection *conn);
S2N_API
extern int s2n_connection_get_session_length(struct s2n_connection *conn);
S2N_API
extern int s2n_connection_get_session_id_length(struct s2n_connection *conn);
S2N_API
extern int s2n_connection_get_session_id(struct s2n_connection *conn, uint8_t *session_id, size_t max_length);
S2N_API
extern int s2n_connection_is_session_resumed(struct s2n_connection *conn);
S2N_API
extern int s2n_connection_is_ocsp_stapled(struct s2n_connection *conn);

/* TLS Signature Algorithms - RFC 5246 7.4.1.4.1 */ 
/* https://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-16 */ 
typedef enum {
    S2N_TLS_SIGNATURE_ANONYMOUS = 0,
    S2N_TLS_SIGNATURE_RSA = 1,
    S2N_TLS_SIGNATURE_ECDSA = 3,

    /* Use Private Range for RSA PSS since it's not defined there */
    S2N_TLS_SIGNATURE_RSA_PSS_RSAE = 224,
    S2N_TLS_SIGNATURE_RSA_PSS_PSS
} s2n_tls_signature_algorithm;

/* TLS Hash Algorithm - https://tools.ietf.org/html/rfc5246#section-7.4.1.4.1 */ 
/* https://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-18 */ 
typedef enum {
    S2N_TLS_HASH_NONE = 0,
    S2N_TLS_HASH_MD5 = 1,
    S2N_TLS_HASH_SHA1 = 2,
    S2N_TLS_HASH_SHA224 = 3,
    S2N_TLS_HASH_SHA256 = 4,
    S2N_TLS_HASH_SHA384 = 5,
    S2N_TLS_HASH_SHA512 = 6,
    
    /* Use Private Range for MD5_SHA1 */
    S2N_TLS_HASH_MD5_SHA1 = 224
} s2n_tls_hash_algorithm;

S2N_API
extern int s2n_connection_get_selected_signature_algorithm(struct s2n_connection *conn, s2n_tls_signature_algorithm *chosen_alg);
S2N_API
extern int s2n_connection_get_selected_digest_algorithm(struct s2n_connection *conn, s2n_tls_hash_algorithm *chosen_alg);
S2N_API
extern int s2n_connection_get_selected_client_cert_signature_algorithm(struct s2n_connection *conn, s2n_tls_signature_algorithm *chosen_alg);
S2N_API
extern int s2n_connection_get_selected_client_cert_digest_algorithm(struct s2n_connection *conn, s2n_tls_hash_algorithm *chosen_alg);

S2N_API
extern struct s2n_cert_chain_and_key *s2n_connection_get_selected_cert(struct s2n_connection *conn);

/**
 * Returns the length of the s2n certificate chain `chain_and_key`.
 * 
 * @param chain_and_key A pointer to the s2n_cert_chain_and_key object being read.
 * @param cert_length This return value represents the length of the s2n certificate chain `chain_and_key`.
 */
S2N_API
extern int s2n_cert_chain_get_length(const struct s2n_cert_chain_and_key *chain_and_key, uint32_t *cert_length);

/**
 * Returns the certificate `out_cert` present at the index `cert_idx` of the certificate chain `chain_and_key`.
 * 
 * Note that the index of the leaf certificate is zero. If the certificate chain `chain_and_key` is NULL or the
 * certificate index value is not in the acceptable range for the input certificate chain, an error is returned.
 * 
 * # Safety 
 *
 * There is no memory allocation required for `out_cert` buffer prior to calling the `s2n_cert_chain_get_cert` API.
 * The `out_cert` will contain the pointer to the s2n_cert initialized within the input s2n_cert_chain_and_key `chain_and_key`.
 * The pointer to the output s2n certificate `out_cert` is valid until `chain_and_key` is freed up. 
 * If a caller wishes to persist the `out_cert` beyond the lifetime of `chain_and_key`, the contents would need to be
 * copied prior to freeing `chain_and_key`.
 *
 * @param chain_and_key A pointer to the s2n_cert_chain_and_key object being read.
 * @param out_cert A pointer to the output s2n_cert `out_cert` present at the index `cert_idx` of the certificate chain `chain_and_key`.
 * @param cert_idx The certificate index for the requested certificate within the s2n certificate chain.
 */
S2N_API
extern int s2n_cert_chain_get_cert(const struct s2n_cert_chain_and_key *chain_and_key, struct s2n_cert **out_cert, const uint32_t cert_idx);

/**
 * Returns the s2n certificate in DER format along with its length.
 * 
 * The API gets the s2n certificate `cert` in DER format. The certificate is returned in the `out_cert_der` buffer.
 * Here, `cert_len` represents the length of the certificate.
 * 
 * A caller can use certificate parsing tools such as the ones provided by OpenSSL to parse the DER encoded certificate chain returned.
 *
 * # Safety
 * 
 * The memory for the `out_cert_der` buffer is allocated and owned by s2n-tls. 
 * Since the size of the certificate can potentially be very large, a pointer to internal connection data is returned instead of 
 * copying the contents into a caller-provided buffer.
 * 
 * The pointer to the output buffer `out_cert_der` is valid only while the connection exists.
 * The `s2n_connection_free` API frees the memory assosciated with the out_cert_der buffer and after the `s2n_connection_wipe` API is
 * called the memory pointed by out_cert_der is invalid.
 * 
 * If a caller wishes to persist the `out_cert_der` beyond the lifetime of the connection, the contents would need to be
 * copied prior to the connection termination.
 * 
 * @param cert A pointer to the s2n_cert object being read.
 * @param out_cert_der A pointer to the output buffer which will hold the s2n certificate `cert` in DER format.
 * @param cert_length This return value represents the length of the certificate.
 */
S2N_API
extern int s2n_cert_get_der(const struct s2n_cert *cert, const uint8_t **out_cert_der, uint32_t *cert_length);

/**
 * Returns the validated peer certificate chain as a `s2n_cert_chain_and_key` opaque object.
 * 
 * The `s2n_cert_chain_and_key` parameter must be allocated by the caller using the `s2n_cert_chain_and_key_new` API
 * prior to this function call and must be empty. To free the memory associated with the `s2n_cert_chain_and_key` object use the 
 * `s2n_cert_chain_and_key_free` API.
 * 
 * @param conn A pointer to the s2n_connection object being read.
 * @param s2n_cert_chain_and_key The returned validated peer certificate chain `cert_chain` retrieved from the s2n connection.
 */
S2N_API
extern int s2n_connection_get_peer_cert_chain(const struct s2n_connection *conn, struct s2n_cert_chain_and_key *cert_chain);

/**
 * Returns the length of the DER encoded extension value of the ASN.1 X.509 certificate extension.
 * 
 * @param cert A pointer to the s2n_cert object being read.
 * @param oid A null-terminated cstring that contains the OID of the X.509 certificate extension to be read.
 * @param ext_value_len This return value contains the length of DER encoded extension value of the ASN.1 X.509 certificate extension.
 */
S2N_API 
extern int s2n_cert_get_x509_extension_value_length(struct s2n_cert *cert, const uint8_t *oid, uint32_t *ext_value_len);

/**
 * Returns the DER encoding of an ASN.1 X.509 certificate extension value, it's length and a boolean critical.
 * 
 * @param cert A pointer to the s2n_cert object being read.
 * @param oid A null-terminated cstring that contains the OID of the X.509 certificate extension to be read.
 * @param ext_value A pointer to the output buffer which will hold the DER encoding of an ASN.1 X.509 certificate extension value returned. 
 * @param ext_value_len  This value is both an input and output parameter and represents the length of the output buffer `ext_value`. 
 * When used as an input parameter, the caller must use this parameter to convey the maximum length of `ext_value`. 
 * When used as an output parameter, `ext_value_len` holds the actual length of the DER encoding of the ASN.1 X.509 certificate extension value returned. 
 * @param critical This return value contains the boolean value for `critical`.
 */
S2N_API 
extern int s2n_cert_get_x509_extension_value(struct s2n_cert *cert, const uint8_t *oid, uint8_t *ext_value, uint32_t *ext_value_len, bool *critical);

/**
 * Returns the UTF8 String length of the ASN.1 X.509 certificate extension data. 
 * 
 * @param extension_data A pointer to the DER encoded ASN.1 X.509 certificate extension value being read.
 * @param extension_len represents the length of the input buffer `extension_data`.
 * @param utf8_str_len This return value contains the UTF8 String length of the ASN.1 X.509 certificate extension data.
 */
S2N_API 
extern int s2n_cert_get_utf8_string_from_extension_data_length(const uint8_t *extension_data, uint32_t extension_len, uint32_t *utf8_str_len);

/**
 * Returns the UTF8 String representation of the DER encoded ASN.1 X.509 certificate extension data.
 * 
 * @param extension_data A pointer to the DER encoded ASN.1 X.509 certificate extension value being read.
 * @param extension_len represents the length of the input buffer `extension_data`.
 * @param out_data A pointer to the output buffer which will hold the UTF8 String representation of the DER encoded ASN.1 X.509 
 * certificate extension data returned. 
 * @param out_len This value is both an input and output parameter and represents the length of the output buffer `out_data`.
 * When used as an input parameter, the caller must use this parameter to convey the maximum length of `out_data`. 
 * When used as an output parameter, `out_len` holds the actual length of UTF8 String returned.
 */
S2N_API 
extern int s2n_cert_get_utf8_string_from_extension_data(const uint8_t *extension_data, uint32_t extension_len, uint8_t *out_data, uint32_t *out_len);

S2N_API
extern uint64_t s2n_connection_get_wire_bytes_in(struct s2n_connection *conn);
S2N_API
extern uint64_t s2n_connection_get_wire_bytes_out(struct s2n_connection *conn);
S2N_API
extern int s2n_connection_get_client_protocol_version(struct s2n_connection *conn);
S2N_API
extern int s2n_connection_get_server_protocol_version(struct s2n_connection *conn);
S2N_API
extern int s2n_connection_get_actual_protocol_version(struct s2n_connection *conn);
S2N_API
extern int s2n_connection_get_client_hello_version(struct s2n_connection *conn);
S2N_API
extern int s2n_connection_client_cert_used(struct s2n_connection *conn);
S2N_API
extern const char *s2n_connection_get_cipher(struct s2n_connection *conn);

/**
 * Returns the IANA value for the connection's negotiated cipher suite.
 *
 * The value is returned in the form of `first,second`, in order to closely match
 * the values defined in the [IANA Registry](https://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#table-tls-parameters-4).
 * For example if the connection's negotiated cipher suite is `TLS_AES_128_GCM_SHA256`,
 * which is registered as `0x13,0x01`, then `first = 0x13` and `second = 0x01`.
 *
 * This method will only succeed after the cipher suite has been negotiated with the peer.
 *
 * @param conn A pointer to the connection being read
 * @param first A pointer to a single byte, which will be updated with the first byte in the registered IANA value.
 * @param second A pointer to a single byte, which will be updated with the second byte in the registered IANA value.
 * @return A POSIX error signal. If an error was returned, the values contained in `first` and `second` should be considered invalid.
 */
S2N_API
extern int s2n_connection_get_cipher_iana_value(struct s2n_connection *conn, uint8_t *first, uint8_t *second);

S2N_API
extern int s2n_connection_is_valid_for_cipher_preferences(struct s2n_connection *conn, const char *version);
S2N_API
extern const char *s2n_connection_get_curve(struct s2n_connection *conn);
S2N_API
extern const char *s2n_connection_get_kem_name(struct s2n_connection *conn);
S2N_API
extern const char *s2n_connection_get_kem_group_name(struct s2n_connection *conn);
S2N_API
extern int s2n_connection_get_alert(struct s2n_connection *conn);
S2N_API
extern const char *s2n_connection_get_handshake_type_name(struct s2n_connection *conn);
S2N_API
extern const char *s2n_connection_get_last_message_name(struct s2n_connection *conn);

struct s2n_async_pkey_op;
typedef enum { S2N_ASYNC_PKEY_VALIDATION_FAST, S2N_ASYNC_PKEY_VALIDATION_STRICT } s2n_async_pkey_validation_mode;
typedef enum { S2N_ASYNC_DECRYPT, S2N_ASYNC_SIGN } s2n_async_pkey_op_type;

typedef int (*s2n_async_pkey_fn)(struct s2n_connection *conn, struct s2n_async_pkey_op *op);
S2N_API
extern int s2n_config_set_async_pkey_callback(struct s2n_config *config, s2n_async_pkey_fn fn);
S2N_API
extern int s2n_async_pkey_op_perform(struct s2n_async_pkey_op *op, s2n_cert_private_key *key);
S2N_API
extern int s2n_async_pkey_op_apply(struct s2n_async_pkey_op *op, struct s2n_connection *conn);
S2N_API
extern int s2n_async_pkey_op_free(struct s2n_async_pkey_op *op);
S2N_API
extern int s2n_config_set_async_pkey_validation_mode(struct s2n_config *config, s2n_async_pkey_validation_mode mode);

S2N_API
extern int s2n_async_pkey_op_get_op_type(struct s2n_async_pkey_op *op, s2n_async_pkey_op_type *type);
S2N_API
extern int s2n_async_pkey_op_get_input_size(struct s2n_async_pkey_op *op, uint32_t *data_len);
S2N_API
extern int s2n_async_pkey_op_get_input(struct s2n_async_pkey_op *op, uint8_t *data, uint32_t data_len);
S2N_API
extern int s2n_async_pkey_op_set_output(struct s2n_async_pkey_op *op, const uint8_t *data, uint32_t data_len);

/**
 * Callback function for handling key log events
 *
 * THIS SHOULD BE USED FOR DEBUGGING PURPOSES ONLY!
 *
 * Each log line is formatted with the
 * [NSS Key Log Format](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSS/Key_Log_Format)
 * without a newline.
 *
 * # Safety
 *
 * * `ctx` MUST be cast into the same type of pointer that was originally created
 * * `logline` bytes MUST be copied or discarded before this function returns
 *
 * @param ctx Context for the callback
 * @param conn Connection for which the log line is being emitted
 * @param logline Pointer to the log line data
 * @param len Length of the log line data
 */
typedef int (*s2n_key_log_fn)(void *ctx, struct s2n_connection *conn, uint8_t *logline, size_t len);

/**
 * Sets a key logging callback on the provided config
 *
 * THIS SHOULD BE USED FOR DEBUGGING PURPOSES ONLY!
 *
 * Setting this function enables configurations to emit secrets in the
 * [NSS Key Log Format](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSS/Key_Log_Format)
 *
 * # Safety
 *
 * * `callback` MUST cast `ctx` into the same type of pointer that was originally created
 * * `ctx` MUST live for at least as long as it is set on the config
 *
 * @param config Config to set the callback
 * @param callback The function that should be called for each secret log entry
 * @param ctx The context to be passed when the callback is called
 */
S2N_API
extern int s2n_config_set_key_log_cb(struct s2n_config *config, s2n_key_log_fn callback, void *ctx);

/* s2n_config_enable_cert_req_dss_legacy_compat adds a dss cert type in the server certificate request when being called.
 * It only sends the dss cert type in the cert request but does not succeed the handshake if a dss cert is received.
 * Please DO NOT call this api unless you know you actually need legacy DSS certificate type compatibility
 */
S2N_API
extern int s2n_config_enable_cert_req_dss_legacy_compat(struct s2n_config *config);

#ifdef __cplusplus
}
#endif
