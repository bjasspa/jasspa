/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * spell.c - Spell checking routines.
 *
 * Copyright (C) 2020-2024 JASSPA (www.jasspa.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define meSOCK_USE_DLL 1
#if meSOCK_USE_DLL
#define OPENSSLFunc(f) sslF_##f
#else
#define OPENSSLFunc(f) f
#endif
#define MESOCK_STRINGQUT(str) #str
#define MESOCK_STRINGIFY(str) MESOCK_STRINGQUT(str)

#include "mesock.h"
#include <time.h>
#include <ctype.h>
#if MEOPT_OPENSSL
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/conf.h>
#include <openssl/engine.h>
#endif

#ifdef _WIN32
#define meBadSocket        INVALID_SOCKET
#define meSockLibGetFunc   GetProcAddress
#define meSockGetError     GetLastError
#define meSocketGetError   WSAGetLastError
#define meSocketOpen       socket
#define meSocketConnect    connect
#define meSocketClose      closesocket
#define meSocketSetOpt     setsockopt
#define meSocketRead       recv
#define meSocketWrite      send
#define meSocketShutdown   shutdown
#define meGetservbyname    getservbyname
#define meGethostbyname    gethostbyname
#define meInet_addr        inet_addr
#define meHtons            htons
#define meClock            GetTickCount
#define snprintf           sprintf_s
static int WSAinit=0;
#else
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/un.h>
#define meBadSocket        -1
#define meSockLibGetFunc   dlsym
#define meSockGetError()   (errno)
#define meSocketGetError() (errno)
#define meSocketOpen       socket
#define meSocketConnect    connect
#define meSocketClose      close
#define meSocketSetOpt     setsockopt
#define meSocketRead       recv
#define meSocketWrite      send
#define meSocketShutdown   shutdown
#define meGetservbyname    getservbyname
#define meGethostbyname    gethostbyname
#define meInet_addr        inet_addr
#define meHtons            htons
#define meClock            clock
#endif

#if MEOPT_OPENSSL

#if meSOCK_USE_DLL

typedef BIO *(*meSOCKF_BIO_new)(const BIO_METHOD *type);
typedef int (*meSOCKF_BIO_free)(BIO *a);
typedef uint64_t (*meSOCKF_BIO_number_written)(BIO *bio);
typedef int (*meSOCKF_BIO_read)(BIO *b, void *data, int dlen);
typedef const BIO_METHOD *(*meSOCKF_BIO_s_mem)(void);
typedef void (*meSOCKF_CRYPTO_free)(void *ptr, const char *file, int line);
typedef void (*meSOCKF_ERR_clear_error)(void);
typedef char *(*meSOCKF_ERR_error_string)(unsigned long e, char *buf);
typedef unsigned long (*meSOCKF_ERR_get_error)(void);
typedef int (*meSOCKF_OPENSSL_sk_num)(const OPENSSL_STACK *);
typedef void *(*meSOCKF_OPENSSL_sk_value)(const OPENSSL_STACK *, int);
typedef void (*meSOCKF_OPENSSL_sk_pop_free)(OPENSSL_STACK *st, void (*func) (void *));
typedef const char *(*meSOCKF_RAND_file_name)(char *file, size_t num);
typedef int (*meSOCKF_RAND_load_file)(const char *file, long max_bytes);
typedef int (*meSOCKF_RAND_status)(void);
typedef const char *(*meSOCKF_SSL_CIPHER_get_name)(const SSL_CIPHER *c);
typedef long (*meSOCKF_SSL_CTX_ctrl)(SSL_CTX *ctx, int cmd, long larg, void *parg);
typedef void (*meSOCKF_SSL_CTX_free)(SSL_CTX *);
typedef X509_STORE *(*meSOCKF_SSL_CTX_get_cert_store)(const SSL_CTX *);
typedef int (*meSOCKF_SSL_CTX_load_verify_locations)(SSL_CTX *ctx, const char *CAfile, const char *CApath);
typedef SSL_CTX *(*meSOCKF_SSL_CTX_new)(const SSL_METHOD *meth);
typedef int (*meSOCKF_SSL_CTX_set1_param)(SSL_CTX *ctx, X509_VERIFY_PARAM *vpm);
typedef int (*meSOCKF_SSL_CTX_set_default_verify_paths)(SSL_CTX *ctx);
typedef uint64_t (*meSOCKF_SSL_CTX_set_options)(SSL_CTX *ctx,uint64_t opts);
typedef void (*meSOCKF_SSL_CTX_set_verify)(SSL_CTX *ctx, int mode, SSL_verify_cb callback);
typedef int (*meSOCKF_SSL_connect)(SSL *ssl);
typedef long (*meSOCKF_SSL_ctrl)(SSL *ssl, int cmd, long larg, void *parg);
typedef void (*meSOCKF_SSL_free)(SSL *ssl);
typedef const SSL_CIPHER *(*meSOCKF_SSL_get_current_cipher)(const SSL *s);
typedef int (*meSOCKF_SSL_get_error)(const SSL *s, int ret_code);
typedef X509 *(*meSOCKF_SSL_get_peer_certificate)(const SSL *s);
typedef long (*meSOCKF_SSL_get_verify_result)(const SSL *ssl);
typedef const char *(*meSOCKF_SSL_get_version)(const SSL *s);
typedef SSL *(*meSOCKF_SSL_new)(SSL_CTX *ctx);
typedef int (*meSOCKF_SSL_read)(SSL *ssl, void *buf, int num);
typedef int (*meSOCKF_SSL_set_fd)(SSL *s, int fd);
typedef int (*meSOCKF_SSL_shutdown)(SSL *s);
typedef int (*meSOCKF_SSL_write)(SSL *ssl, const void *buf, int num);
typedef SSL_SESSION *(*meSOCKF_SSL_get_session)(SSL *s);
typedef int (*meSOCKF_SSL_set_session)(SSL *s,SSL_SESSION *n);
typedef const SSL_METHOD *(*meSOCKF_TLS_client_method)(void);
typedef int (*meSOCKF_X509_NAME_print_ex)(BIO *out, const X509_NAME *nm, int indent, unsigned long flags);
typedef int (*meSOCKF_X509_STORE_add_cert)(X509_STORE *ctx, X509 *x);
typedef void (*meSOCKF_X509_VERIFY_PARAM_free)(X509_VERIFY_PARAM *param);
typedef X509_VERIFY_PARAM *(*meSOCKF_X509_VERIFY_PARAM_new)(void);
typedef int (*meSOCKF_X509_VERIFY_PARAM_set_flags)(X509_VERIFY_PARAM *param, unsigned long flags);
typedef void *(*meSOCKF_X509_get_ext_d2i)(const X509 *x, int nid, int *crit, int *idx);
typedef X509_NAME *(*meSOCKF_X509_get_issuer_name)(const X509 *a);
typedef X509_NAME *(*meSOCKF_X509_get_subject_name)(const X509 *a);
typedef const char *(*meSOCKF_X509_verify_cert_error_string)(long n);
typedef ASN1_OCTET_STRING *(*meSOCKF_a2i_IPADDRESS)(const char *ipasc);
typedef ASN1_STRING *(*meSOCKF_X509_NAME_ENTRY_get_data)(const X509_NAME_ENTRY *ne);
typedef X509_NAME_ENTRY *(*meSOCKF_X509_NAME_get_entry)(const X509_NAME *name, int loc);
typedef int (*meSOCKF_X509_NAME_get_index_by_NID)(X509_NAME *name, int nid, int lastpos);
typedef int (*meSOCKF_ASN1_STRING_length)(const ASN1_STRING *x);
typedef char *(*meSOCKF_X509_NAME_oneline)(const X509_NAME *a, char *buf, int size);
typedef int (*meSOCKF_X509_NAME_print_ex_fp)(FILE *fp, const X509_NAME *nm, int indent, unsigned long flags);
typedef int (*meSOCKF_ASN1_STRING_cmp)(const ASN1_STRING *a, const ASN1_STRING *b);
typedef int (*meSOCKF_ASN1_STRING_to_UTF8)(unsigned char **out, const ASN1_STRING *in);
typedef int (*meSOCKF_CONF_modules_load_file)(const char *filename, const char *appname, unsigned long flags);
typedef void (*meSOCKF_ENGINE_load_builtin_engines)(void);
typedef int (*meSOCKF_OPENSSL_init_ssl)(uint64_t opts, const OPENSSL_INIT_SETTINGS *settings);
typedef void (*meSOCKF_OPENSSL_load_builtin_modules)(void);
typedef int (*meSOCKF_RAND_poll)(void);
typedef const SSL_METHOD *(*meSOCKF_TLS_client_method)(void);
typedef long (*meSOCKF_SSL_get_verify_result)(const SSL *ssl);
typedef void (*meSOCKF_X509_free)(X509 *a);
typedef void (*meSOCKF_GENERAL_NAME_free)(GENERAL_NAME *a);
typedef void (*meSOCKF_CRYPTO_free)(void *ptr, const char *file, int line);
typedef void (*meSOCKF_ASN1_OCTET_STRING_free)(ASN1_OCTET_STRING *a);
typedef X509 *(*meSOCKF_d2i_X509)(X509 **a, const unsigned char **in, long len);
typedef int (*meSOCKF_X509_NAME_get_text_by_NID)(X509_NAME *name, int nid, char *buf, int len);

meSOCKF_ASN1_OCTET_STRING_free sslF_ASN1_OCTET_STRING_free;
meSOCKF_ASN1_STRING_cmp sslF_ASN1_STRING_cmp;
meSOCKF_ASN1_STRING_length sslF_ASN1_STRING_length;
meSOCKF_ASN1_STRING_to_UTF8 sslF_ASN1_STRING_to_UTF8;
meSOCKF_BIO_free sslF_BIO_free;
meSOCKF_BIO_new sslF_BIO_new;
meSOCKF_BIO_number_written sslF_BIO_number_written;
meSOCKF_BIO_read sslF_BIO_read;
meSOCKF_BIO_s_mem sslF_BIO_s_mem;
meSOCKF_CONF_modules_load_file sslF_CONF_modules_load_file;
meSOCKF_CRYPTO_free sslF_CRYPTO_free;
meSOCKF_CRYPTO_free sslF_CRYPTO_free;
meSOCKF_ENGINE_load_builtin_engines sslF_ENGINE_load_builtin_engines;
meSOCKF_ERR_clear_error sslF_ERR_clear_error;
meSOCKF_ERR_error_string sslF_ERR_error_string;
meSOCKF_ERR_get_error sslF_ERR_get_error;
meSOCKF_GENERAL_NAME_free sslF_GENERAL_NAME_free;
meSOCKF_OPENSSL_init_ssl sslF_OPENSSL_init_ssl;
meSOCKF_OPENSSL_load_builtin_modules sslF_OPENSSL_load_builtin_modules;
meSOCKF_OPENSSL_sk_num sslF_OPENSSL_sk_num;
meSOCKF_OPENSSL_sk_value sslF_OPENSSL_sk_value;
meSOCKF_OPENSSL_sk_pop_free sslF_OPENSSL_sk_pop_free;
meSOCKF_RAND_file_name sslF_RAND_file_name;
meSOCKF_RAND_load_file sslF_RAND_load_file;
meSOCKF_RAND_poll sslF_RAND_poll;
meSOCKF_RAND_status sslF_RAND_status;
meSOCKF_SSL_CIPHER_get_name sslF_SSL_CIPHER_get_name;
meSOCKF_SSL_CTX_ctrl sslF_SSL_CTX_ctrl;
meSOCKF_SSL_CTX_free sslF_SSL_CTX_free;
meSOCKF_SSL_CTX_get_cert_store sslF_SSL_CTX_get_cert_store;
meSOCKF_SSL_CTX_load_verify_locations sslF_SSL_CTX_load_verify_locations;
meSOCKF_SSL_CTX_new sslF_SSL_CTX_new;
meSOCKF_SSL_CTX_set1_param sslF_SSL_CTX_set1_param;
meSOCKF_SSL_CTX_set_default_verify_paths sslF_SSL_CTX_set_default_verify_paths;
meSOCKF_SSL_CTX_set_options sslF_SSL_CTX_set_options;
meSOCKF_SSL_CTX_set_verify sslF_SSL_CTX_set_verify;
meSOCKF_SSL_connect sslF_SSL_connect;
meSOCKF_SSL_ctrl sslF_SSL_ctrl;
meSOCKF_SSL_free sslF_SSL_free;
meSOCKF_SSL_get_current_cipher sslF_SSL_get_current_cipher;
meSOCKF_SSL_get_error sslF_SSL_get_error;
meSOCKF_SSL_get_peer_certificate sslF_SSL_get_peer_certificate;
meSOCKF_SSL_get_verify_result sslF_SSL_get_verify_result;
meSOCKF_SSL_get_verify_result sslF_SSL_get_verify_result;
meSOCKF_SSL_get_version sslF_SSL_get_version;
meSOCKF_SSL_new sslF_SSL_new;
meSOCKF_SSL_read sslF_SSL_read;
meSOCKF_SSL_set_fd sslF_SSL_set_fd;
meSOCKF_SSL_shutdown sslF_SSL_shutdown;
meSOCKF_SSL_write sslF_SSL_write;
meSOCKF_SSL_get_session sslF_SSL_get_session;
meSOCKF_SSL_set_session sslF_SSL_set_session;
meSOCKF_TLS_client_method sslF_TLS_client_method;
meSOCKF_TLS_client_method sslF_TLS_client_method;
meSOCKF_X509_NAME_ENTRY_get_data sslF_X509_NAME_ENTRY_get_data;
meSOCKF_X509_NAME_get_entry sslF_X509_NAME_get_entry;
meSOCKF_X509_NAME_get_index_by_NID sslF_X509_NAME_get_index_by_NID;
meSOCKF_X509_NAME_oneline sslF_X509_NAME_oneline;
meSOCKF_X509_NAME_print_ex sslF_X509_NAME_print_ex;
meSOCKF_X509_NAME_print_ex_fp sslF_X509_NAME_print_ex_fp;
meSOCKF_X509_STORE_add_cert sslF_X509_STORE_add_cert;
meSOCKF_X509_VERIFY_PARAM_free sslF_X509_VERIFY_PARAM_free;
meSOCKF_X509_VERIFY_PARAM_new sslF_X509_VERIFY_PARAM_new;
meSOCKF_X509_VERIFY_PARAM_set_flags sslF_X509_VERIFY_PARAM_set_flags;
meSOCKF_X509_free sslF_X509_free;
meSOCKF_X509_get_ext_d2i sslF_X509_get_ext_d2i;
meSOCKF_X509_get_issuer_name sslF_X509_get_issuer_name;
meSOCKF_X509_get_subject_name sslF_X509_get_subject_name;
meSOCKF_X509_verify_cert_error_string sslF_X509_verify_cert_error_string;
meSOCKF_a2i_IPADDRESS sslF_a2i_IPADDRESS;
meSOCKF_d2i_X509 sslF_d2i_X509;
meSOCKF_X509_NAME_get_text_by_NID sslF_X509_NAME_get_text_by_NID;

static ossl_unused ossl_inline int sslF_sk_GENERAL_NAME_num(const STACK_OF(GENERAL_NAME) *sk)
{
    return OPENSSLFunc(OPENSSL_sk_num)((const OPENSSL_STACK *)sk);
}
static ossl_unused ossl_inline GENERAL_NAME *sslF_sk_GENERAL_NAME_value(const STACK_OF(GENERAL_NAME) *sk, int idx)
{
    return (GENERAL_NAME *) OPENSSLFunc(OPENSSL_sk_value)((const OPENSSL_STACK *)sk, idx);
}
static ossl_unused ossl_inline void sslF_sk_GENERAL_NAME_pop_free(STACK_OF(GENERAL_NAME) *sk, sk_GENERAL_NAME_freefunc freefunc)
{
    OPENSSLFunc(OPENSSL_sk_pop_free)((OPENSSL_STACK *)sk, (OPENSSL_sk_freefunc)freefunc);
}
#endif

static SSL_CTX *meSockCtx=NULL;
#endif
static meUByte *proxyHost=NULL;
static int proxyPort=0;
static meSockLogger logFunc;
static void *logData;
static meUByte *ioBuff=NULL;
static int ioBuffSz=0;
static int timeoutSnd=0;
static int timeoutRcv=0;

static void
strBase64Encode3(meUByte *dd, meUByte c1, meUByte c2, meUByte c3)
{
    static meUByte base64Table[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" ;
    dd[0] = base64Table[(c1 >> 2)] ;
    dd[1] = base64Table[((c1 << 4) & 0x30) | ((c2 >> 4) & 0x0f)] ;
    dd[2] = base64Table[((c2 << 2) & 0x3c) | ((c3 >> 6) & 0x03)] ;
    dd[3] = base64Table[(c3 & 0x3f)] ;
}

static meUByte *
strBase64Encode(meUByte *dd, meUByte *ss)
{
    meUByte c1, c2, c3 ;

    while((c1=*ss++) != '\0')
    {
        if((c2=*ss++) == '\0')
        {
            c3 = '\0' ;
            strBase64Encode3(dd,c1,c2,c3) ;
            dd += 2 ;
            *dd++ = '=' ;
            *dd++ = '=' ;
            break ;
        }
        else if((c3=*ss++) == '\0')
        {
            strBase64Encode3(dd,c1,c2,c3) ;
            dd += 3 ;
            *dd++ = '=' ;
            break ;
        }
        strBase64Encode3(dd,c1,c2,c3) ;
        dd += 4 ;
    }
    *dd = '\0' ;
    return dd ;
}

#if MEOPT_OPENSSL
static void
meSockGetSSLErrors(meUShort logFlags, meUByte *buff)
{
    if(logFlags & meSOCK_LOG_WARNING)
    {
        unsigned long err;
        while((err = OPENSSLFunc(ERR_get_error)()) != 0)
        {
            snprintf((char *) buff,meSOCK_BUFF_SIZE,"  OpenSSL: %s",OPENSSLFunc(ERR_error_string)(err,NULL));
            logFunc(meSOCK_LOG_WARNING,buff,logData);
        }
    }
    else
        OPENSSLFunc(ERR_clear_error)();
}

static int
meSockInitPrng(meUShort logFlags, meUByte *buff)
{
    const char *random_file;

    if(OPENSSLFunc(RAND_status)())
        return 0;

    /* Get the random file name using RAND_file_name. */
    buff[0] = '\0';
    if(((random_file = OPENSSLFunc(RAND_file_name)((char *) buff,meSOCK_BUFF_SIZE)) != NULL) && (random_file[0] != '\0'))
    {
        /* Seed at most 16k (apparently arbitrary value borrowed from
           curl) from random file. */
        OPENSSLFunc(RAND_load_file)(random_file,16384);
        if(OPENSSLFunc(RAND_status)())
            return 0;
    }
    OPENSSLFunc(RAND_poll)();
    if(OPENSSLFunc(RAND_status)())
        return 0;
    if(logFlags & meSOCK_LOG_ERROR)
    {
        snprintf((char *) buff,meSOCK_BUFF_SIZE,"meSock Error: Could not seed PRNG");
        logFunc(meSOCK_LOG_ERROR,buff,logData);
    }
    meSockGetSSLErrors(logFlags,buff);
    return -1;
}

static int
meSockIsValidIp4Address(const char *str, const char *end)
{
    int saw_digit = 0;
    int octets = 0;
    int val = 0;

    while (str < end)
    {
        int ch = *str++;

        if (ch >= '0' && ch <= '9')
        {
            val = val * 10 + (ch - '0');

            if (val > 255)
                return 0;
            if (!saw_digit)
            {
                if (++octets > 4)
                    return 0;
                saw_digit = 1;
            }
        }
        else if (ch == '.' && saw_digit)
        {
            if (octets == 4)
                return 0;
            val = 0;
            saw_digit = 0;
        }
        else
            return 0;
    }
    if (octets < 4)
        return 0;

    return 1;
}

#ifdef ENABLE_IPV6
static int
meSockIsValidIp6Address(const char *str, const char *end)
{
    /* Use lower-case for these to avoid clash with system headers.  */
    enum {
        ns_inaddrsz  = 4,
        ns_in6addrsz = 16,
        ns_int16sz   = 2
    };

    const char *curtok;
    int tp;
    const char *colonp;
    int saw_xdigit;
    unsigned int val;

    tp = 0;
    colonp = NULL;

    if (str == end)
        return 0;

    /* Leading :: requires some special handling. */
    if (*str == ':')
    {
        ++str;
        if (str == end || *str != ':')
            return 0;
    }

    curtok = str;
    saw_xdigit = 0;
    val = 0;

    while (str < end)
    {
        int ch = *str++;

        /* if ch is a number, add it to val. */
        if (c_isxdigit (ch))
        {
            val <<= 4;
            val |= XDIGIT_TO_NUM (ch);
            if (val > 0xffff)
                return 0;
            saw_xdigit = 1;
            continue;
        }

        /* if ch is a colon ... */
        if (ch == ':')
        {
            curtok = str;
            if (!saw_xdigit)
            {
                if (colonp != NULL)
                    return 0;
                colonp = str + tp;
                continue;
            }
            else if (str == end)
                return 0;
            if (tp > ns_in6addrsz - ns_int16sz)
                return 0;
            tp += ns_int16sz;
            saw_xdigit = 0;
            val = 0;
            continue;
        }

        /* if ch is a dot ... */
        if (ch == '.' && (tp <= ns_in6addrsz - ns_inaddrsz)
            && meSockIsValidIp4Address(curtok, end) == 1)
        {
            tp += ns_inaddrsz;
            saw_xdigit = 0;
            break;
        }

        return 0;
    }

    if (saw_xdigit)
    {
        if (tp > ns_in6addrsz - ns_int16sz)
            return 0;
        tp += ns_int16sz;
    }

    if (colonp != NULL)
    {
        if (tp == ns_in6addrsz)
            return 0;
        tp = ns_in6addrsz;
    }

    if (tp != ns_in6addrsz)
        return 0;

    return 1;
}
#endif

int
meSockIsValidIpAddress(const char *name)
{
    const char *endp;

    endp = name + strlen(name);
    if(meSockIsValidIp4Address(name,endp))
        return 1;
#ifdef ENABLE_IPV6
    if(meSockIsValidIp6Address (name, endp))
        return 1;
#endif
    return 0;
}
static void
meSockHostnameGetSni(const char *hostname, char *outBuff)
{
    size_t len = strlen(hostname);
    memcpy(outBuff,hostname,len);

  /* Remove trailing dot(s) to fix #47408.
   * Regarding RFC 6066 (SNI): The hostname is represented as a byte
   * string using ASCII encoding without a trailing dot. */
    while((len > 0) && (outBuff[len-1] == '.'))
        len--;
    outBuff[len] = 0;
}


#ifdef _WIN32
void
meSockInitSystemCaCert(SSL_CTX *sslCtx, meUShort logFlags, char *rbuff)
{
    HCERTSTORE hStore;
    PCCERT_CONTEXT cCntx = NULL;
    const unsigned char *ce;
    X509 *x509;
    X509_STORE *store;
    char buf[128];
    int ii=1;

    if((store = OPENSSLFunc(SSL_CTX_get_cert_store)(sslCtx)) == NULL)
    {
        if(logFlags & meSOCK_LOG_WARNING)
            logFunc(meSOCK_LOG_WARNING,(meUByte *) "OpenSSL: Failed to get SSL_CTX cert store",logData);
        return;
    }

    do {
        // The Root store holds the main root CAs and the CA store holds any intermediate CAs - bail out if we can't open the Root
        if((hStore = CertOpenSystemStore(0,(ii) ? "Root":"CA")) == 0)
        {
            if(logFlags & meSOCK_LOG_WARNING)
                logFunc(meSOCK_LOG_WARNING,(meUByte *) "OpenSSL: Failed to open Root system cert store",logData);
            return;
        }

        while((cCntx = CertEnumCertificatesInStore(hStore,cCntx)) != NULL)
        {
            //uncomment the line below if you want to see the certificates as pop ups
            //CryptUIDlgViewContext(CERT_STORE_CERTIFICATE_CONTEXT,cCntx,NULL,NULL,0,NULL);
            ce = cCntx->pbCertEncoded;
            x509 = OPENSSLFunc(d2i_X509)(NULL,&ce,cCntx->cbCertEncoded);
            if (x509)
            {
                OPENSSLFunc(X509_NAME_oneline)(OPENSSLFunc(X509_get_subject_name)(x509),buf,sizeof(buf));
                if(OPENSSLFunc(X509_STORE_add_cert)(store,x509) == 1)
                {
                    if(logFlags & meSOCK_LOG_VERBOSE)
                    {
                        snprintf(rbuff,meSOCK_BUFF_SIZE,"OpenSSL: Loaded & added Windows Root certificate for: subject='%s'",buf);
                        logFunc(meSOCK_LOG_VERBOSE,(meUByte *) rbuff,logData);
                    }
                }
                else if(logFlags & meSOCK_LOG_WARNING)
                {
                    snprintf(rbuff,meSOCK_BUFF_SIZE,"OpenSSL: Loaded but failed to added Windows Root certificate for: subject='%s'",buf);
                    logFunc(meSOCK_LOG_WARNING,(meUByte *) rbuff,logData);
                }
                OPENSSLFunc(X509_free)(x509);
            }
            else if(logFlags & meSOCK_LOG_WARNING)
                logFunc(meSOCK_LOG_WARNING,(meUByte *) "OpenSSL: Failed to generate x509 for Windows Root certificate",logData);
        }
        CertCloseStore(hStore,0);
    } while(--ii >= 0);
}
#endif

int
meSockInit(meUShort logFlags, meUByte *buff)
{
#if meSOCK_USE_DLL
#ifdef _WIN32
    HINSTANCE libHandle ;
#else
    void *libHandle ;
#endif
#endif
    SSL_METHOD const *meth;
#ifdef X509_V_FLAG_PARTIAL_CHAIN
    X509_VERIFY_PARAM *param;
#endif
    if(meSockCtx != NULL)
        /* The SSL has already been initialized. */
        return 1;

#if meSOCK_USE_DLL
    if(logFlags & meSOCK_LOG_VERBOSE)
        logFunc(meSOCK_LOG_VERBOSE,(meUByte *) "About to get OpenSSL functions",logData);
#ifdef _WIN32
    libHandle = LoadLibrary(MESOCK_STRINGIFY(_OPENSSLCNM));
#else
    libHandle = dlopen(MESOCK_STRINGIFY(_OPENSSLCNM),RTLD_LOCAL|RTLD_LAZY);
#endif
    if(libHandle == NULL)
    {
        if(logFlags & meSOCK_LOG_ERROR)
        {
            snprintf((char *) buff,meSOCK_BUFF_SIZE,"meSock Error: Failed to load " MESOCK_STRINGIFY(_OPENSSLCNM) " library - OpenSSL installed? (%d)",meSockGetError());
            logFunc(meSOCK_LOG_ERROR,buff,logData);
        }
        return -1;
    }
    if(((sslF_ASN1_OCTET_STRING_free = (meSOCKF_ASN1_OCTET_STRING_free) meSockLibGetFunc(libHandle,"ASN1_OCTET_STRING_free")) == NULL) ||
       ((sslF_ASN1_STRING_cmp = (meSOCKF_ASN1_STRING_cmp) meSockLibGetFunc(libHandle,"ASN1_STRING_cmp")) == NULL) ||
       ((sslF_ASN1_STRING_length = (meSOCKF_ASN1_STRING_length) meSockLibGetFunc(libHandle,"ASN1_STRING_length")) == NULL) ||
       ((sslF_ASN1_STRING_to_UTF8 = (meSOCKF_ASN1_STRING_to_UTF8) meSockLibGetFunc(libHandle,"ASN1_STRING_to_UTF8")) == NULL) ||
       ((sslF_BIO_free = (meSOCKF_BIO_free) meSockLibGetFunc(libHandle,"BIO_free")) == NULL) ||
       ((sslF_BIO_new = (meSOCKF_BIO_new) meSockLibGetFunc(libHandle,"BIO_new")) == NULL) ||
       ((sslF_BIO_number_written = (meSOCKF_BIO_number_written) meSockLibGetFunc(libHandle,"BIO_number_written")) == NULL) ||
       ((sslF_BIO_read = (meSOCKF_BIO_read) meSockLibGetFunc(libHandle,"BIO_read")) == NULL) ||
       ((sslF_BIO_s_mem = (meSOCKF_BIO_s_mem) meSockLibGetFunc(libHandle,"BIO_s_mem")) == NULL) ||
       ((sslF_CONF_modules_load_file = (meSOCKF_CONF_modules_load_file) meSockLibGetFunc(libHandle,"CONF_modules_load_file")) == NULL) ||
       ((sslF_CRYPTO_free = (meSOCKF_CRYPTO_free) meSockLibGetFunc(libHandle,"CRYPTO_free")) == NULL) ||
       ((sslF_ENGINE_load_builtin_engines = (meSOCKF_ENGINE_load_builtin_engines) meSockLibGetFunc(libHandle,"ENGINE_load_builtin_engines")) == NULL) ||
       ((sslF_ERR_clear_error = (meSOCKF_ERR_clear_error) meSockLibGetFunc(libHandle,"ERR_clear_error")) == NULL) ||
       ((sslF_ERR_error_string = (meSOCKF_ERR_error_string) meSockLibGetFunc(libHandle,"ERR_error_string")) == NULL) ||
       ((sslF_ERR_get_error = (meSOCKF_ERR_get_error) meSockLibGetFunc(libHandle,"ERR_get_error")) == NULL) ||
       ((sslF_GENERAL_NAME_free = (meSOCKF_GENERAL_NAME_free) meSockLibGetFunc(libHandle,"GENERAL_NAME_free")) == NULL) ||
       ((sslF_OPENSSL_load_builtin_modules = (meSOCKF_OPENSSL_load_builtin_modules) meSockLibGetFunc(libHandle,"OPENSSL_load_builtin_modules")) == NULL) ||
       ((sslF_OPENSSL_sk_num = (meSOCKF_OPENSSL_sk_num) meSockLibGetFunc(libHandle,"OPENSSL_sk_num")) == NULL) ||
       ((sslF_OPENSSL_sk_pop_free = (meSOCKF_OPENSSL_sk_pop_free) meSockLibGetFunc(libHandle,"OPENSSL_sk_pop_free")) == NULL) ||
       ((sslF_OPENSSL_sk_value = (meSOCKF_OPENSSL_sk_value) meSockLibGetFunc(libHandle,"OPENSSL_sk_value")) == NULL) ||
       ((sslF_RAND_file_name = (meSOCKF_RAND_file_name) meSockLibGetFunc(libHandle,"RAND_file_name")) == NULL) ||
       ((sslF_RAND_load_file = (meSOCKF_RAND_load_file) meSockLibGetFunc(libHandle,"RAND_load_file")) == NULL) ||
       ((sslF_RAND_poll = (meSOCKF_RAND_poll) meSockLibGetFunc(libHandle,"RAND_poll")) == NULL) ||
       ((sslF_RAND_status = (meSOCKF_RAND_status) meSockLibGetFunc(libHandle,"RAND_status")) == NULL) ||
       ((sslF_X509_NAME_ENTRY_get_data = (meSOCKF_X509_NAME_ENTRY_get_data) meSockLibGetFunc(libHandle,"X509_NAME_ENTRY_get_data")) == NULL) ||
       ((sslF_X509_NAME_get_entry = (meSOCKF_X509_NAME_get_entry) meSockLibGetFunc(libHandle,"X509_NAME_get_entry")) == NULL) ||
       ((sslF_X509_NAME_get_index_by_NID = (meSOCKF_X509_NAME_get_index_by_NID) meSockLibGetFunc(libHandle,"X509_NAME_get_index_by_NID")) == NULL) ||
       ((sslF_X509_NAME_get_text_by_NID = (meSOCKF_X509_NAME_get_text_by_NID) meSockLibGetFunc(libHandle,"X509_NAME_get_text_by_NID")) == NULL) ||
       ((sslF_X509_NAME_oneline = (meSOCKF_X509_NAME_oneline) meSockLibGetFunc(libHandle,"X509_NAME_oneline")) == NULL) ||
       ((sslF_X509_NAME_print_ex = (meSOCKF_X509_NAME_print_ex) meSockLibGetFunc(libHandle,"X509_NAME_print_ex")) == NULL) ||
       ((sslF_X509_NAME_print_ex_fp = (meSOCKF_X509_NAME_print_ex_fp) meSockLibGetFunc(libHandle,"X509_NAME_print_ex_fp")) == NULL) ||
       ((sslF_X509_STORE_add_cert = (meSOCKF_X509_STORE_add_cert) meSockLibGetFunc(libHandle,"X509_STORE_add_cert")) == NULL) ||
       ((sslF_X509_VERIFY_PARAM_free = (meSOCKF_X509_VERIFY_PARAM_free) meSockLibGetFunc(libHandle,"X509_VERIFY_PARAM_free")) == NULL) ||
       ((sslF_X509_VERIFY_PARAM_new = (meSOCKF_X509_VERIFY_PARAM_new) meSockLibGetFunc(libHandle,"X509_VERIFY_PARAM_new")) == NULL) ||
       ((sslF_X509_VERIFY_PARAM_set_flags = (meSOCKF_X509_VERIFY_PARAM_set_flags) meSockLibGetFunc(libHandle,"X509_VERIFY_PARAM_set_flags")) == NULL) ||
       ((sslF_X509_free = (meSOCKF_X509_free) meSockLibGetFunc(libHandle,"X509_free")) == NULL) ||
       ((sslF_X509_get_ext_d2i = (meSOCKF_X509_get_ext_d2i) meSockLibGetFunc(libHandle,"X509_get_ext_d2i")) == NULL) ||
       ((sslF_X509_get_issuer_name = (meSOCKF_X509_get_issuer_name) meSockLibGetFunc(libHandle,"X509_get_issuer_name")) == NULL) ||
       ((sslF_X509_get_subject_name = (meSOCKF_X509_get_subject_name) meSockLibGetFunc(libHandle,"X509_get_subject_name")) == NULL) ||
       ((sslF_X509_verify_cert_error_string = (meSOCKF_X509_verify_cert_error_string) meSockLibGetFunc(libHandle,"X509_verify_cert_error_string")) == NULL) ||
       ((sslF_a2i_IPADDRESS = (meSOCKF_a2i_IPADDRESS) meSockLibGetFunc(libHandle,"a2i_IPADDRESS")) == NULL) ||
       ((sslF_d2i_X509 = (meSOCKF_d2i_X509) meSockLibGetFunc(libHandle,"d2i_X509")) == NULL))
    {
        if(logFlags & meSOCK_LOG_ERROR)
        {
            snprintf((char *) buff,meSOCK_BUFF_SIZE,"meSock Error: Failed to load libcrypto functions (%d)",meSockGetError());
            logFunc(meSOCK_LOG_ERROR,buff,logData);
        }
        return -2;
    }

#ifdef _WIN32
    libHandle = LoadLibrary(MESOCK_STRINGIFY(_OPENSSLLNM));
#else
    libHandle = dlopen(MESOCK_STRINGIFY(_OPENSSLLNM),RTLD_LOCAL|RTLD_LAZY);
#endif
    if(libHandle == NULL)
    {
        snprintf((char *) buff,meSOCK_BUFF_SIZE,"meSock Error: Failed to load " MESOCK_STRINGIFY(_OPENSSLLNM) " library - OpenSSL installed? (%d)",meSockGetError());
        if(logFlags & meSOCK_LOG_ERROR)
            logFunc(meSOCK_LOG_ERROR,buff,logData);
        return -3;
    }
    if(((sslF_OPENSSL_init_ssl = (meSOCKF_OPENSSL_init_ssl) meSockLibGetFunc(libHandle,"OPENSSL_init_ssl")) == NULL) ||
       ((sslF_SSL_CIPHER_get_name = (meSOCKF_SSL_CIPHER_get_name) meSockLibGetFunc(libHandle,"SSL_CIPHER_get_name")) == NULL) ||
       ((sslF_SSL_CTX_ctrl = (meSOCKF_SSL_CTX_ctrl) meSockLibGetFunc(libHandle,"SSL_CTX_ctrl")) == NULL) ||
       ((sslF_SSL_CTX_free = (meSOCKF_SSL_CTX_free) meSockLibGetFunc(libHandle,"SSL_CTX_free")) == NULL) ||
       ((sslF_SSL_CTX_get_cert_store = (meSOCKF_SSL_CTX_get_cert_store) meSockLibGetFunc(libHandle,"SSL_CTX_get_cert_store")) == NULL) ||
       ((sslF_SSL_CTX_load_verify_locations = (meSOCKF_SSL_CTX_load_verify_locations) meSockLibGetFunc(libHandle,"SSL_CTX_load_verify_locations")) == NULL) ||
       ((sslF_SSL_CTX_new = (meSOCKF_SSL_CTX_new) meSockLibGetFunc(libHandle,"SSL_CTX_new")) == NULL) ||
       ((sslF_SSL_CTX_set1_param = (meSOCKF_SSL_CTX_set1_param) meSockLibGetFunc(libHandle,"SSL_CTX_set1_param")) == NULL) ||
       ((sslF_SSL_CTX_set_default_verify_paths = (meSOCKF_SSL_CTX_set_default_verify_paths) meSockLibGetFunc(libHandle,"SSL_CTX_set_default_verify_paths")) == NULL) ||
       ((sslF_SSL_CTX_set_options = (meSOCKF_SSL_CTX_set_options) meSockLibGetFunc(libHandle,"SSL_CTX_set_options")) == NULL) ||
       ((sslF_SSL_CTX_set_verify = (meSOCKF_SSL_CTX_set_verify) meSockLibGetFunc(libHandle,"SSL_CTX_set_verify")) == NULL) ||
       ((sslF_SSL_connect = (meSOCKF_SSL_connect) meSockLibGetFunc(libHandle,"SSL_connect")) == NULL) ||
       ((sslF_SSL_ctrl = (meSOCKF_SSL_ctrl) meSockLibGetFunc(libHandle,"SSL_ctrl")) == NULL) ||
       ((sslF_SSL_free = (meSOCKF_SSL_free) meSockLibGetFunc(libHandle,"SSL_free")) == NULL) ||
       ((sslF_SSL_get_current_cipher = (meSOCKF_SSL_get_current_cipher) meSockLibGetFunc(libHandle,"SSL_get_current_cipher")) == NULL) ||
       ((sslF_SSL_get_error = (meSOCKF_SSL_get_error) meSockLibGetFunc(libHandle,"SSL_get_error")) == NULL) ||
       ((sslF_SSL_get_verify_result = (meSOCKF_SSL_get_verify_result) meSockLibGetFunc(libHandle,"SSL_get_verify_result")) == NULL) ||
       ((sslF_SSL_get_version = (meSOCKF_SSL_get_version) meSockLibGetFunc(libHandle,"SSL_get_version")) == NULL) ||
       ((sslF_SSL_new = (meSOCKF_SSL_new) meSockLibGetFunc(libHandle,"SSL_new")) == NULL) ||
       ((sslF_SSL_read = (meSOCKF_SSL_read) meSockLibGetFunc(libHandle,"SSL_read")) == NULL) ||
       ((sslF_SSL_set_fd = (meSOCKF_SSL_set_fd) meSockLibGetFunc(libHandle,"SSL_set_fd")) == NULL) ||
       ((sslF_SSL_shutdown = (meSOCKF_SSL_shutdown) meSockLibGetFunc(libHandle,"SSL_shutdown")) == NULL) ||
       ((sslF_SSL_write = (meSOCKF_SSL_write) meSockLibGetFunc(libHandle,"SSL_write")) == NULL) ||
       ((sslF_SSL_get_session = (meSOCKF_SSL_get_session) meSockLibGetFunc(libHandle,"SSL_get_session")) == NULL) ||
       ((sslF_SSL_set_session = (meSOCKF_SSL_set_session) meSockLibGetFunc(libHandle,"SSL_set_session")) == NULL) ||
       ((sslF_TLS_client_method = (meSOCKF_TLS_client_method) meSockLibGetFunc(libHandle,"TLS_client_method")) == NULL) ||
       (((sslF_SSL_get_peer_certificate = (meSOCKF_SSL_get_peer_certificate) meSockLibGetFunc(libHandle,"SSL_get1_peer_certificate")) == NULL) &&
        ((sslF_SSL_get_peer_certificate = (meSOCKF_SSL_get_peer_certificate) meSockLibGetFunc(libHandle,"SSL_get_peer_certificate")) == NULL)) ||
       0)
    {
        if(logFlags & meSOCK_LOG_ERROR)
        {
            snprintf((char *) buff,meSOCK_BUFF_SIZE,"meSock Error: Failed to load libssl functions (%d)",meSockGetError());
            logFunc(meSOCK_LOG_ERROR,buff,logData);
        }
        return -4;
    }
#endif

    /* Init the PRNG.  If that fails, bail out.  */
    if(meSockInitPrng(logFlags,buff) < 0)
        return -7;

    OPENSSLFunc(OPENSSL_load_builtin_modules)();
#ifndef OPENSSL_NO_ENGINE
    OPENSSLFunc(ENGINE_load_builtin_engines)();
#endif
    OPENSSLFunc(CONF_modules_load_file)(NULL,NULL,CONF_MFLAGS_DEFAULT_SECTION|CONF_MFLAGS_IGNORE_MISSING_FILE);
    OPENSSLFunc(OPENSSL_init_ssl)(0, NULL);

    meth = OPENSSLFunc(TLS_client_method)();
    meSockCtx = OPENSSLFunc(SSL_CTX_new)(meth);
    if(!meSockCtx)
    {
        if(logFlags & meSOCK_LOG_ERROR)
            logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: Failed to create SSL context",logData);
        meSockGetSSLErrors(logFlags,buff);
        return -8;
    }

    OPENSSLFunc(SSL_CTX_set_default_verify_paths)(meSockCtx);
    OPENSSLFunc(SSL_CTX_load_verify_locations)(meSockCtx, NULL,NULL);
#ifdef _WIN32
    // load windows system ca certs
    meSockInitSystemCaCert(meSockCtx,logFlags,(char *) buff);
#endif

#ifdef X509_V_FLAG_PARTIAL_CHAIN
    /* Set X509_V_FLAG_PARTIAL_CHAIN to allow the client to anchor trust in
     * a non-self-signed certificate. This defies RFC 4158 (Path Building)
     * which defines a trust anchor in terms of a self-signed certificate.
     * However, it substantially reduces attack surface by pruning the tree
     * of unneeded trust points. For example, the cross-certified
     * Let's Encrypt X3 CA, which protects gnu.org and appears as an
     * intermediate CA to clients, can be used as a trust anchor without
     * the entire IdentTrust PKI.
     */
    param = OPENSSLFunc(X509_VERIFY_PARAM_new)();
    if (param)
    {
        /* We only want X509_V_FLAG_PARTIAL_CHAIN, but the OpenSSL docs
         * say to use X509_V_FLAG_TRUSTED_FIRST also. It looks like
         * X509_V_FLAG_TRUSTED_FIRST applies to a collection of trust
         * anchors and not a single trust anchor.
         */
        (void) OPENSSLFunc(X509_VERIFY_PARAM_set_flags)(param, X509_V_FLAG_TRUSTED_FIRST | X509_V_FLAG_PARTIAL_CHAIN);
        if(OPENSSLFunc(SSL_CTX_set1_param)(meSockCtx, param) == 0)
        {
            if(logFlags & meSOCK_LOG_WARNING)
                logFunc(meSOCK_LOG_WARNING,(meUByte *) "meSock Warning: Failed set trust to partial chain",logData);
        }
        /* We continue on error */
        OPENSSLFunc(X509_VERIFY_PARAM_free)(param);
    }
    else if(logFlags & meSOCK_LOG_WARNING)
    {
        logFunc(meSOCK_LOG_WARNING,(meUByte *) "meSock Warning: Failed to allocate verification param",logData);
        /* We continue on error */
    }
#endif

    /* SSL_VERIFY_NONE instructs OpenSSL not to abort SSL_connect if the certificate is invalid. We
     * verify the certificate separately in meSockCheckCertificate, which provides much better
     * diagnostics than examining the error stack after a failed SSL_connect. */
    OPENSSLFunc(SSL_CTX_set_verify)(meSockCtx, SSL_VERIFY_NONE, NULL);


    /* The OpenSSL library can handle renegotiations automatically, so tell it to do so. */
    OPENSSLFunc(SSL_CTX_ctrl)(meSockCtx,SSL_CTRL_MODE,SSL_MODE_AUTO_RETRY,NULL);

#ifdef SSL_OP_IGNORE_UNEXPECTED_EOF
    /* IIS can still send non-chunked responses without a content-length, this triggers an EOF error in v3+ so ignore for now, could turn into an option */
    OPENSSLFunc(SSL_CTX_set_options)(meSockCtx,SSL_OP_IGNORE_UNEXPECTED_EOF);
#endif    
    return 0;
}

#define ASTERISK_EXCLUDES_DOT   /* mandated by rfc2818 */

/* Return true is STRING (case-insensitively) matches PATTERN, false otherwise. The recognized
 * wildcard character is "*", which matches any character in STRING except ".". Any number of the
 * "*" wildcard may be present in the pattern.
 *
 * This is used to match of hosts as indicated in rfc2818:
 *
 *      Names may contain the wildcard character * which is considered to match any single domain
 *      name component or component fragment. E.g., *.a.com matches foo.a.com but not bar.foo.a.com.
 *      f*.com matches foo.com but not bar.com [or foo.bar.com].
 *
 * If the pattern contain no wildcards, pattern_match(a, b) is equivalent to !strcasecmp(a, b).
 */

static int
meSockPatternMatch(const char *pattern, const char *string)
{
    const char *p = pattern, *n = string;
    char c;
    for (; (c = tolower(*p++)) != '\0'; n++)
    {
        if (c == '*')
        {
            for (c = tolower(*p); c == '*'; c = tolower(*++p))
                ;
            for (; *n != '\0'; n++)
            {
                if (tolower(*n) == c && meSockPatternMatch(p, n))
                    return 1;
#ifdef ASTERISK_EXCLUDES_DOT
                else if (*n == '.')
                    return 0;
#endif
            }
            return (c == '\0');
        }
        else if (c != tolower(*n))
            return 0;
    }
    return *n == '\0';
}

static char *
meSockGetRfc2253FormattedName(X509_NAME *name)
{
    int len;
    char *out=NULL;
    BIO* b;

    if((b = OPENSSLFunc(BIO_new)(OPENSSLFunc(BIO_s_mem)())))
    {
        if((OPENSSLFunc(X509_NAME_print_ex)(b,name,0,XN_FLAG_RFC2253) >= 0) &&
           ((len = (int) OPENSSLFunc(BIO_number_written)(b)) > 0))
        {
            out = malloc(len+1);
            OPENSSLFunc(BIO_read)(b,out,len);
            out[len] = 0;
        }
        OPENSSLFunc(BIO_free)(b);
    }

    return (out == NULL) ? strdup(""):out;
}

int
meSockCheckCertificate(meSockFile *sFp, const char *sniHost, meUByte *rbuff)
{
    X509 *cert;
    GENERAL_NAMES *subjectAltNames;
    char common_name[256];
    long vresult;
    int ret, alt_name_checked = 0;
    const char *severityLbl;
    meUByte logType;
#if 0
    int pinsuccess = opt.pinnedpubkey == NULL;
    /* The user explicitly said to not check for the certificate.  */
    if ((sFp->flags == meSOCK_IGN_CRT_ERR) && pinsuccess)
        return success;
#else
    if(sFp->flags == meSOCK_IGN_CRT_ERR)
        /* don't error and no logging - so stop now */
        return 1;
#endif
    logType = (sFp->flags & meSOCK_IGN_CRT_ERR) ? meSOCK_LOG_WARNING:meSOCK_LOG_ERROR;
    if((logFunc != NULL) && (sFp->flags & logType))
        severityLbl = (sFp->flags & meSOCK_IGN_CRT_ERR) ? "Warning":"Error";
    else
        severityLbl = NULL;
    if((cert = OPENSSLFunc(SSL_get_peer_certificate)(sFp->ssl)) == NULL)
    {
        if(severityLbl != NULL)
        {
            snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock %s: No certificate presented by %s",severityLbl,sniHost);
            logFunc(logType,rbuff,logData);
        }
        return (sFp->flags & meSOCK_IGN_CRT_ERR) ? 1:0;
    }

    if(sFp->flags & meSOCK_LOG_VERBOSE)
    {
        char *subject = meSockGetRfc2253FormattedName(OPENSSLFunc(X509_get_subject_name)(cert));
        char *issuer = meSockGetRfc2253FormattedName(OPENSSLFunc(X509_get_issuer_name)(cert));
        snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock - Certificate:\n  subject: %s\n  issuer:  %s",subject,issuer);
        logFunc(meSOCK_LOG_VERBOSE,rbuff,logData);
        free(subject);
        free(issuer);
    }

    ret = 1;
    vresult = OPENSSLFunc(SSL_get_verify_result)(sFp->ssl);
    if (vresult != X509_V_OK)
    {
        if(severityLbl != NULL)
        {
            char *dd, *issuer = meSockGetRfc2253FormattedName(OPENSSLFunc(X509_get_issuer_name)(cert));
            /* Try to print more user-friendly (and translated) messages for
               the frequent verification errors.  */
            switch (vresult)
            {
            case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
                dd = "Unable to locally verify the issuer's authority";
                break;
            case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
            case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
                dd = "Self-signed certificate encountered";
                break;
            case X509_V_ERR_CERT_NOT_YET_VALID:
                dd = "Issued certificate not yet valid";
                break;
            case X509_V_ERR_CERT_HAS_EXPIRED:
                dd = "Issued certificate has expired";
                break;
            default:
                /* For the less frequent error strings, simply provide the OpenSSL error message. */
                if((dd = (char *) OPENSSLFunc(X509_verify_cert_error_string)(vresult)) == NULL)
                    dd = "Unknown";
            }
            snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock %s: Cannot verify %s's certificate, issued by %s:  %s",severityLbl,sniHost,issuer,dd);
            free(issuer);
            logFunc(logType,rbuff,logData);
        }
        ret = 0;
        /* Fall through, so that the user is warned about *all* issues
           with the cert (important with --no-check-certificate.)  */
    }

    /* Check that HOST matches the common name in the certificate. The following remains to be
     * done:
     *
     * - When matching against common names, it should loop over all common names and choose the
     *   most specific one, i.e. the last one, not the first one, which the current code picks.
     *
     * - Ensure that ASN1 strings from the certificate are encoded as UTF-8 which can be
     *   meaningfully compared to HOST.
     */
    if((subjectAltNames = OPENSSLFunc(X509_get_ext_d2i)(cert,NID_subject_alt_name,NULL,NULL)) != NULL)
    {
        /* Test subject alternative names */
        /* Do we want to check for dNSNames or ipAddresses (see RFC 2818)?
         * Signal it by host_in_octet_string. */
        ASN1_OCTET_STRING *host_in_octet_string = OPENSSLFunc(a2i_IPADDRESS)(sniHost);

        int numaltnames = OPENSSLFunc(sk_GENERAL_NAME_num)(subjectAltNames);
        int ii;
        for(ii=0; ii<numaltnames; ii++)
        {
            const GENERAL_NAME *name = OPENSSLFunc(sk_GENERAL_NAME_value)(subjectAltNames,ii);
            if (name)
            {
                if (host_in_octet_string)
                {
                    if (name->type == GEN_IPADD)
                    {
                        /* Check for ipAddress */
                        /* TODO: Should we convert between IPv4-mapped IPv6 addresses and IPv4 addresses? */
                        alt_name_checked = 1;
                        if (!OPENSSLFunc(ASN1_STRING_cmp)(host_in_octet_string,name->d.iPAddress))
                            break;
                    }
                }
                else if (name->type == GEN_DNS)
                {
                    /* dNSName should be IA5String (i.e. ASCII), however who does trust CA? Convert
                     * it into UTF-8 for sure. */
                    unsigned char *name_in_utf8 = NULL;

                    /* Check for dNSName */
                    alt_name_checked = 1;

                    if (0 <= OPENSSLFunc(ASN1_STRING_to_UTF8)(&name_in_utf8, name->d.dNSName))
                    {
                        /* Compare and check for NULL attack in ASN1_STRING */
                        if (meSockPatternMatch((char *) name_in_utf8,sniHost) &&
                            (strlen((char *) name_in_utf8) == (size_t) OPENSSLFunc(ASN1_STRING_length)(name->d.dNSName)))
                        {
                            OPENSSLFunc(CRYPTO_free)(name_in_utf8,OPENSSL_FILE,OPENSSL_LINE);
                            break;
                        }
                        OPENSSLFunc(CRYPTO_free)(name_in_utf8,OPENSSL_FILE,OPENSSL_LINE);
                    }
                }
            }
        }
        OPENSSLFunc(sk_GENERAL_NAME_pop_free)(subjectAltNames,OPENSSLFunc(GENERAL_NAME_free));
        if(host_in_octet_string)
            OPENSSLFunc(ASN1_OCTET_STRING_free)(host_in_octet_string);

        if((alt_name_checked == 1) && (ii >= numaltnames))
        {
            if(severityLbl != NULL)
            {
                snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock %s: No certificate subject alternative name matches requested host name %s",severityLbl,sniHost);
                logFunc(logType,rbuff,logData);
            }
            ret = 0;
        }
    }

    if (alt_name_checked == 0)
    {
        /* Test commomName */
        X509_NAME *xname = OPENSSLFunc(X509_get_subject_name)(cert);
        common_name[0] = '\0';
        OPENSSLFunc(X509_NAME_get_text_by_NID)(xname,NID_commonName,common_name,sizeof(common_name));

        if (!meSockPatternMatch(common_name,sniHost))
        {
            if(severityLbl != NULL)
            {
                snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock %s: Certificate common name %s doesn't match requested host name %s",severityLbl,common_name,sniHost);
                logFunc(logType,rbuff,logData);
            }
            ret = 0;
        }
        else
        {
            /* We now determine the length of the ASN1 string. If it differs from common_name's
             * length, then there is a \0 before the string terminates. This can be an instance of a
             * null-prefix attack.
             *
             * https://www.blackhat.com/html/bh-usa-09/bh-usa-09-archives.html#Marlinspike
             */
            int ii=-1, jj;
            X509_NAME_ENTRY *xentry;
            ASN1_STRING *sdata;

            if(xname)
            {
                for (;;)
                {
                    jj = OPENSSLFunc(X509_NAME_get_index_by_NID)(xname, NID_commonName, ii);
                    if(jj == -1)
                        break;
                    ii = jj;
                }
            }

            xentry = OPENSSLFunc(X509_NAME_get_entry)(xname,ii);
            sdata = OPENSSLFunc(X509_NAME_ENTRY_get_data)(xentry);
            if (strlen(common_name) != (size_t) OPENSSLFunc(ASN1_STRING_length)(sdata))
            {
                if(severityLbl != NULL)
                {
                    snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock %s: Certificate common name is invalid (contains a NUL character).\n  This may be an indication that the host is not who it claims to be\n  (that is, it is not the real %s).",severityLbl,sniHost);
                    logFunc(logType,rbuff,logData);
                }
                ret = 0;
            }
        }
    }

    if(ret && (sFp->flags & meSOCK_LOG_VERBOSE))
    {
        snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Verbose: X509 certificate successfully verified and matches host %s",sniHost);
        logFunc(meSOCK_LOG_VERBOSE,rbuff,logData);
    }
    OPENSSLFunc(X509_free)(cert);

    return (sFp->flags & meSOCK_IGN_CRT_ERR) ? 1:ret;
}
#endif

static int
meSockReadLine(meSockFile *sFp, meUByte *buff)
{
    meUByte *dd=buff, cc ;
    meInt ret, err;
    for(;;)
    {
#if MEOPT_OPENSSL
        if(sFp->flags & meSOCK_USE_SSL)
            ret = OPENSSLFunc(SSL_read)(sFp->ssl,dd,1);
        else
#endif
            ret = meSocketRead(sFp->sck,(char *) dd,1,0);
        if(ret <= 0)
        {
#if MEOPT_OPENSSL
            if(sFp->flags & meSOCK_USE_SSL)
                err = OPENSSLFunc(SSL_get_error)(sFp->ssl,ret);
            else
#endif
                err = (ret == 0) ? 0:meSocketGetError();
            if(sFp->flags & meSOCK_LOG_ERROR)
            {
                snprintf((char *) buff,meSOCK_BUFF_SIZE,"meSock Error: Failed to read line - %d,%d",ret,err);
                logFunc(meSOCK_LOG_ERROR,buff,logData);
            }
#if MEOPT_OPENSSL
            if(sFp->flags & meSOCK_USE_SSL)
            {
                if((err == SSL_ERROR_SYSCALL) || (err == SSL_ERROR_SSL))
                    sFp->flags &= ~meSOCK_SHUTDOWN;
                meSockGetSSLErrors(sFp->flags,buff);
            }
#endif
            sFp->flags |= meSOCK_CLOSE;
            return -1;
        }
        if((cc=*dd) == '\n')
            break ;
        if(cc != '\r')
            dd++;
    }
    *dd = '\0';
    return 1;
}


void
meSockSetup(meSockLogger logger,void *data, int toSnd, int toRcv, int ioBfSz, meUByte *ioBf)
{
    logFunc = logger;
    logData = data;
    timeoutSnd = toSnd;
    timeoutRcv = toRcv;
    ioBuff = ioBf;
    ioBuffSz = ioBfSz;
#ifdef _WIN32
    if(!WSAinit)
    {
        WSADATA wsaData;
        if(!WSAStartup(MAKEWORD(1,1),&wsaData))
            WSAinit = 1;
        else if(logFunc != NULL)
            logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: WSAStartup failed",logData);
    }
#endif
}

void
meSockSetProxy(meUByte *host, meInt port)
{
    if(proxyHost != NULL)
    {
        if((host == NULL) || strcmp((char *) host,(char *) proxyHost))
        {
            free(proxyHost);
            proxyHost = NULL;
        }
        else
            host = NULL;
    }
    if(host != NULL)
        proxyHost = (meUByte *) strdup((char *) host);
    proxyPort = port;
}

int
meSockHttpOpen(meSockFile *sFp, meUShort flags, meUByte *host, meInt port, meUByte *user, meUByte *pass, meUByte *file, meCookie *cookie,
               meInt fdLen, meUByte *frmData, meUByte *postFName, meUByte *rbuff)
{
    meUByte *ss;
    meInt pfs, ret, err, ll;
    FILE *pfp=NULL;
    meSOCKET sck;
#if MEOPT_OPENSSL
    SSL *ssl;
#endif

    if(logFunc == NULL)
        flags &= ~(meSOCK_LOG_STATUS|meSOCK_LOG_ERROR|meSOCK_LOG_WARNING|meSOCK_LOG_DETAILS|meSOCK_LOG_VERBOSE);
    flags = (flags & meSOCK_PUBLIC_MASK) | meSOCK_INUSE;

    if(meSockIsInUse(sFp))
    {
        if((sFp->flags & meSOCK_CLOSE) || (sFp->cprt != port) || (sFp->length < 0) || ((sck=sFp->sck) == meBadSocket) ||
           ((sFp->flags & (meSOCK_USE_SSL|meSOCK_CTRL_INUSE)) != (flags & (meSOCK_USE_SSL|meSOCK_CTRL_INUSE))) ||
           strcmp((char *) sFp->chst,(char *) host) ||
           strcmp((char *) sFp->cusr,((user == NULL) ? "":(char *) user)))
            meSockClose(sFp,1);
#ifdef _WIN32
        else
        {
            /* On windows writing to a sever shutdown (half-open) socket does not fail so poll to see if there is anything to read.
             * This works because when the server sends a FIN an attempt to read will succeed with a length of 0 indicating the end, if there
             * actually is something to read we still want to close the socket as things must have gone horribly out of sync!
             * On UNIX this test does not appear to work, however the initial send fails with an EPIPE error - see below. */
            fd_set rfds;
            TIMEVAL tv;
            tv.tv_sec = 0;
            tv.tv_usec = 0;
            FD_ZERO(&rfds);
            FD_SET(sck,&rfds);
            if((select(1,&rfds,NULL,NULL,&tv) == 1) && FD_ISSET(sck,&rfds))
            {
                if(sFp->flags & meSOCK_LOG_DETAILS)
                    logFunc(meSOCK_LOG_DETAILS,(meUByte *) "meSock Info: Closing connection as server shutdown",logData);
                meSockClose(sFp,1);
            }
        }
#endif
    }
    sFp->length = -1;
    sFp->chnkLen = 0;
    if(meSockIsInUse(sFp))
    {
        sFp->flags = (sFp->flags & meSOCK_CTRL_SHUTDOWN) | flags;
#if MEOPT_OPENSSL
        ssl = sFp->ssl;
#else
    }
    else if(flags & meSOCK_USE_SSL)
    {
        if(sFp->flags & meSOCK_LOG_ERROR)
            logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: HTTPS not supported in this build",logData);
        return -24;
#endif
    }
    else
    {
        struct sockaddr_in sa;
        struct hostent *hp;
        meUByte *thost;
        meInt tport;

        sFp->home = NULL;
        sFp->sck = meBadSocket;
        sFp->ctrlSck = meBadSocket;
#if MEOPT_OPENSSL
        sFp->ssl = NULL;
        sFp->ctrlSsl = NULL;
        ssl = NULL;
#endif
        sFp->cprt = -1;
        if(ioBuffSz < meSOCK_IOBUF_MIN)
        {
            if(flags & meSOCK_LOG_ERROR)
                logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: Setup not called with IO buffer",logData);
            return -10;
        }
        if((fdLen > 0) && (postFName != NULL))
        {
            if(flags & meSOCK_LOG_ERROR)
                logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: Cannot use http form data and post file at the same time",logData);
            return -21;
        }
#if MEOPT_OPENSSL
        if(flags & meSOCK_USE_SSL)
        {
            if((meSockCtx == NULL) && ((ret = meSockInit(flags,rbuff)) < 0))
                return ret;
            OPENSSLFunc(ERR_clear_error)();
        }
#endif
        if(flags & meSOCK_LOG_STATUS)
        {
            snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"Connecting to %s:%d",host,(port) ? port:((flags & meSOCK_USE_SSL) ? 443:80));
            logFunc(meSOCK_LOG_STATUS,rbuff,logData);
        }
        if(proxyHost != NULL)
        {
            thost = proxyHost ;
            tport = proxyPort ;
        }
        else
        {
            thost = host ;
            tport = port ;
        }

        if(((hp = gethostbyname((char *) thost)) == NULL) ||
           (hp->h_length > ((int) sizeof(struct in_addr))) ||
           (hp->h_addrtype != AF_INET))
        {
            if(flags & meSOCK_LOG_ERROR)
            {
                snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Error: Failed to get host [%s] %d",thost,(hp == NULL) ? meSocketGetError():-1);
                logFunc(meSOCK_LOG_ERROR,rbuff,logData);
            }
            return -11;
        }
        memset(&sa,0,sizeof(sa));
        sa.sin_family = AF_INET;
        memcpy(&sa.sin_addr.s_addr,hp->h_addr,hp->h_length);
        if(tport)
            sa.sin_port=htons(tport);
        else if(flags & meSOCK_USE_SSL)
            sa.sin_port=htons(443);
        else
            sa.sin_port=htons(80);

#if MEOPT_OPENSSL
        if(flags & meSOCK_USE_SSL)
        {
            if((ssl=OPENSSLFunc(SSL_new)(meSockCtx)) == NULL)
            {
                if(flags & meSOCK_LOG_ERROR)
                    logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSockHttp Error: Failed to create new SSL",logData);
                return -12;
            }
            meSockHostnameGetSni((char *) thost,(char *) ioBuff);
            if(!meSockIsValidIpAddress((char *) ioBuff))
            {
                // SSL_set_tlsext_host_name(ssl,ioBuff);
                long rc = OPENSSLFunc(SSL_ctrl)(ssl,SSL_CTRL_SET_TLSEXT_HOSTNAME,TLSEXT_NAMETYPE_host_name,(void *) ioBuff);
                if(rc == 0)
                {
                    OPENSSLFunc(SSL_free)(ssl);
                    if(flags & meSOCK_LOG_ERROR)
                        logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: Failed to set TLS server-name indication",logData);
                    return -13;
                }
            }
        }
#endif

        if((sck=meSocketOpen(AF_INET,SOCK_STREAM,IPPROTO_TCP)) == meBadSocket)
        {
            if(flags & meSOCK_LOG_ERROR)
                logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: Failed to create socket",logData);
#if MEOPT_OPENSSL
            if(flags & meSOCK_USE_SSL)
                OPENSSLFunc(SSL_free)(ssl);
#endif
            return -14;
        }
        sFp->sck = sck;
#if MEOPT_OPENSSL
        sFp->ssl = ssl;
#endif
        sFp->flags = flags;
        if(timeoutSnd > 0)
        {
            /* using setsockopt to set the send timeout to given millisecs, alternatively use select(0,fd_set,NULL,NULL,struct timeval) */
            struct timeval to;
            to.tv_sec = timeoutSnd;
            to.tv_usec = 0;
            if((ret=setsockopt(sck,SOL_SOCKET,SO_SNDTIMEO,(char *) &to,sizeof(to))) < 0)
            {
                if(sFp->flags & meSOCK_LOG_WARNING)
                {
                    snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Warning: Failed to set send timeout %d,%d",ret,meSocketGetError());
                    logFunc(meSOCK_LOG_WARNING,rbuff,logData);
                }
            }
        }
        if(timeoutRcv > 0)
        {
            struct timeval to;
            to.tv_sec = timeoutRcv;
            to.tv_usec = 0;
            if((ret=setsockopt(sck,SOL_SOCKET,SO_RCVTIMEO,(char *) &to,sizeof(to))) < 0)
            {
                if(sFp->flags & meSOCK_LOG_WARNING)
                {
                    snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Warning: Failed to set recv timeout %d,%d",ret,meSocketGetError());
                    logFunc(meSOCK_LOG_WARNING,rbuff,logData);
                }
            }
        }

#if MEOPT_OPENSSL
        /*Bind the socket to the SSL structure*/
        if((flags & meSOCK_USE_SSL) && ((ret = OPENSSLFunc(SSL_set_fd)(ssl,sck)) < 1))
        {
            err = OPENSSLFunc(SSL_get_error)(ssl,ret);
            if(sFp->flags & meSOCK_LOG_ERROR)
            {
                snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Error: Failed to set SSL fd - error %d,%d",ret,err);
                logFunc(meSOCK_LOG_ERROR,rbuff,logData);
            }
            meSockGetSSLErrors(sFp->flags,rbuff);
            meSockClose(sFp,1);
            return -15;
        }
#endif
        /* Connect to the server, TCP/IP layer,*/
        if(meSocketConnect(sck,(struct sockaddr*) &sa,sizeof(sa)) != 0)
        {
            if(sFp->flags & meSOCK_LOG_ERROR)
            {
                snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Error: Failed to connect socket - error %d",meSocketGetError());
                logFunc(meSOCK_LOG_ERROR,rbuff,logData);
            }
            meSockClose(sFp,1);
            return -16;
        }

#if MEOPT_OPENSSL
        if(flags & meSOCK_USE_SSL)
        {
            /*Connect to the server, SSL layer.*/
            if((ret=OPENSSLFunc(SSL_connect)(ssl)) < 1)
            {
                err = OPENSSLFunc(SSL_get_error)(ssl,ret);
                if(sFp->flags & meSOCK_LOG_ERROR)
                {
                    snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Error: Failed to connect SSL - error %d,%d",ret,err);
                    logFunc(meSOCK_LOG_ERROR,rbuff,logData);
                }
                meSockGetSSLErrors(sFp->flags,rbuff);
                meSockClose(sFp,1);
                return -17;
            }

            /*Print out connection details*/
            if(sFp->flags & meSOCK_LOG_DETAILS)
            {
                snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"SSL connected on socket %x, version: %s, cipher: %s",sck,OPENSSLFunc(SSL_get_version)(ssl),
                         OPENSSLFunc(SSL_CIPHER_get_name)(OPENSSLFunc(SSL_get_current_cipher)(ssl)));
                logFunc(meSOCK_LOG_DETAILS,rbuff,logData);
            }
            if(!meSockCheckCertificate(sFp,(char *) ioBuff,rbuff))
            {
                meSockClose(sFp,1);
                return -18;
            }
            sFp->flags |= meSOCK_SHUTDOWN;
        }
        else
#endif
            if(sFp->flags & meSOCK_LOG_DETAILS)
        {
            snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"Connected on socket %x",sck);
            logFunc(meSOCK_LOG_DETAILS,rbuff,logData);
        }
        sFp->cprt = port;
        strcpy((char *) sFp->chst,(char *) host);
        sFp->cusr[0] = '\0';
    }

    if(sFp->flags & meSOCK_LOG_STATUS)
    {
        memcpy(rbuff,"Connected - downloading ",24);
        ret = strlen((char *) file);
        if(ret > 132)
            ret = 132;
        memcpy(rbuff+24,file,ret);
        strcpy((char *) (rbuff+24+ret),"...");
        logFunc(meSOCK_LOG_STATUS,rbuff,logData);
    }
    /*Send message to the server.*/
    ss = ioBuff;
    if(postFName != NULL)
    {
        if((pfp=fopen((char *) postFName,"rb")) == NULL)
        {
            if(sFp->flags & meSOCK_LOG_ERROR)
            {
                snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Error: Failed to open post file [%s]",postFName);
                logFunc(meSOCK_LOG_ERROR,rbuff,logData);
            }
            meSockClose(sFp,1);
            return -19;
        }
        /* send POST message to http */
        strcpy((char *) ss,"POST ") ;
        ss += 5;
    }
    else if(fdLen > 0)
    {
        /* send POST message to http */
        strcpy((char *) ss,"POST ") ;
        ss += 5;
    }
    else
    {
        /* send GET message to http */
        strcpy((char *) ss,"GET ") ;
        ss += 4;
    }
    if(proxyHost == NULL)
    {
        strcpy((char *) ss,(char *) file);
        ss += strlen((char *) ss) ;
    }
    else
    {
        strcpy((char *) ss,"http") ;
        ss += 4;
        if(flags & meSOCK_USE_SSL)
            *ss++ = 's';
        if(port)
            ss += sprintf((char *) ss,"://%s:%d%s",host,port,file);
        else
            ss += sprintf((char *) ss,"://%s%s",host,file);
    }
    strcpy((char *) ss," HTTP/1.1\r\nHost: ") ;
    ss += 17;
    strcpy((char *) ss,(char *) host) ;
    ss += strlen((char *) ss) ;
    if(port)
        ss += sprintf((char *) ss,":%d",port);
    if(sFp->flags & meSOCK_CLOSE)
    {
        strcpy((char *) ss,"\r\nConnection: close");
        ss += 19;
    }
    else
    {
        strcpy((char *) ss,"\r\nConnection: keep-alive");
        ss += 24;
    }
    if(user != NULL)
    {
        /* password supplied, encode */
        strcpy((char *) ss,"\r\nAuthorization: Basic ") ;
        ss += 23 ;
        sprintf((char *) rbuff,"%s:%s",user,pass);
        ss = strBase64Encode(ss,rbuff) ;
        strcpy((char *) sFp->cusr,(char *) user);
    }
    if(pfp != NULL)
    {
        meInt ii;
        meUByte *s1, *s2 ;
        fseek(pfp,0,SEEK_END);
        ii = ftell(pfp) ;
        fseek(pfp,0,SEEK_SET) ;
        if((s1 = (meUByte *) strrchr((char *) postFName,'/')) == NULL)
            s1 = postFName;
        if((s2 = (meUByte *) strrchr((char *) s1,'\\')) == NULL)
            s2 = s1;
        pfs = sprintf((char *) rbuff,"\r\n----5Iz6dTINmxNFw6S42Ryf98IBXX1NCe%x",(meUInt) meClock());
        ll = sprintf((char *) rbuff+pfs,"\r\nContent-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\nContent-Type: application/octet-stream\r\n\r\n",s2);
        ss += sprintf((char *) ss,"\r\nContent-Length: %d",pfs-2+ll+ii+pfs+4);
    }
    else if(fdLen > 0)
    {
        ss += sprintf((char *) ss,"\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d",fdLen);
    }
    if(cookie != NULL)
    {
        if((cookie->value != NULL) && (cookie->value[0] != '\0'))
        {
            strcpy((char *) ss,"\r\nCookie: ");
            ss += 10;
            strcpy((char *) ss,(char *) cookie->value);
            ss += strlen((char *) ss);
        }
        if(cookie->buffLen < 0)
            cookie = NULL;
    }
    if(sFp->flags & meSOCK_LOG_DETAILS)
    {
        ss[0] = '\n';
        ss[1] = '\0';
        logFunc(meSOCK_LOG_DETAILS,(meUByte *) "HTTP Request header:",logData);
        logFunc(meSOCK_LOG_DETAILS,ioBuff,logData);
    }
    strcpy((char *) ss,"\r\n\r\n");
    ll = ss+4-ioBuff;
    ss = ioBuff;
    for(;;)
    {
#if MEOPT_OPENSSL
        if(flags & meSOCK_USE_SSL)
            ret = OPENSSLFunc(SSL_write)(ssl,ss,ll);
        else
#endif
            ret = meSocketWrite(sck,(char *) ss,ll,0);
        if(ret <= 0)
        {
#if MEOPT_OPENSSL
            if(flags & meSOCK_USE_SSL)
            {
                if(((err = OPENSSLFunc(SSL_get_error)(ssl,ret)) == SSL_ERROR_SYSCALL) || (err == SSL_ERROR_SSL))
                    sFp->flags &= ~meSOCK_SHUTDOWN;
#ifndef _WIN32
                if((err == SSL_ERROR_SYSCALL) && (meSocketGetError() == EPIPE))
                    err = EPIPE;
                else if(err == EPIPE)
                    /* this should never be the case as EPIPE == 32 and highest ssl err is 11, but better safe */
                    err = -1;
#endif
            }
            else
#endif
                err = meSocketGetError();
#ifndef _WIN32
            if(err == EPIPE)
            {
                if(sFp->flags & meSOCK_LOG_DETAILS)
                    logFunc(meSOCK_LOG_DETAILS,(meUByte *) "meSock Info: Closing connection as server shutdown",logData);
                meSockClose(sFp,1);
                return meSockHttpOpen(sFp,flags,host,port,user,pass,file,cookie,fdLen,frmData,postFName,rbuff);
            }
#endif
            if(sFp->flags & meSOCK_LOG_ERROR)
            {
                snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Error: HTTP header write error %d,%d",ret,err);
                logFunc(meSOCK_LOG_ERROR,rbuff,logData);
            }
#if MEOPT_OPENSSL
            if(flags & meSOCK_USE_SSL)
                meSockGetSSLErrors(sFp->flags,rbuff);
#endif
            meSockClose(sFp,1);
            if(pfp != NULL)
                fclose(pfp);
            return -20;
        }
        if((ll -= ret) <= 0)
            break;
        ss += ret;
    }
    if(pfp != NULL)
    {
        ss = rbuff+2;
        ll = strlen((char *) ss);
        for(;;)
        {
#if MEOPT_OPENSSL
            if(flags & meSOCK_USE_SSL)
                ret = OPENSSLFunc(SSL_write)(ssl,ss,ll);
            else
#endif
                ret = meSocketWrite(sck,(char *) ss,ll,0);
            if((ret <= 0) || ((ll -= ret) <= 0))
                break;
            ss += ret;
        }
        if(ret > 0)
        {
            while((ll=fread(ioBuff,1,ioBuffSz,pfp)) > 0)
            {
                ss = ioBuff;
                for(;;)
                {
#if MEOPT_OPENSSL
                    if(flags & meSOCK_USE_SSL)
                        ret = OPENSSLFunc(SSL_write)(ssl,ss,ll);
                    else
#endif
                        ret = meSocketWrite(sck,(char *) ss,ll,0);
                    if((ret <= 0) || ((ll -= ret) <= 0))
                        break;
                    ss += ret;
                }
            }
            fclose(pfp);
            if(ret > 0)
            {
                strcpy((char *) rbuff+pfs,"--\r\n");
#if MEOPT_OPENSSL
                if(flags & meSOCK_USE_SSL)
                    ret = OPENSSLFunc(SSL_write)(ssl,rbuff,pfs+4);
                else
#endif
                    ret = meSocketWrite(sck,(char *) rbuff,pfs+4,0);
            }
        }
        else
            fclose(pfp);
        if(ret <= 0)
        {
#if MEOPT_OPENSSL
            if(flags & meSOCK_USE_SSL)
            {
                if(((err = OPENSSLFunc(SSL_get_error)(ssl,ret)) == SSL_ERROR_SYSCALL) || (err == SSL_ERROR_SSL))
                    sFp->flags &= ~meSOCK_SHUTDOWN;
            }
            else
#endif
                err = meSocketGetError();
            if(sFp->flags & meSOCK_LOG_ERROR)
            {
                snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Error: Post file write error %d,%d",ret,err);
                logFunc(meSOCK_LOG_ERROR,rbuff,logData);
            }
#if MEOPT_OPENSSL
            if(flags & meSOCK_USE_SSL)
                meSockGetSSLErrors(sFp->flags,rbuff);
#endif
            meSockClose(sFp,1);
            return -22;
        }
    }
    else if(fdLen > 0)
    {
        for(;;)
        {
#if MEOPT_OPENSSL
            if(flags & meSOCK_USE_SSL)
                ret = OPENSSLFunc(SSL_write)(ssl,frmData,fdLen);
            else
#endif
                ret = meSocketWrite(sck,(char *) frmData,fdLen,0);
            if(ret <= 0)
            {
#if MEOPT_OPENSSL
                if(flags & meSOCK_USE_SSL)
                {
                    if(((err = OPENSSLFunc(SSL_get_error)(ssl,ret)) == SSL_ERROR_SYSCALL) || (err == SSL_ERROR_SSL))
                        sFp->flags &= ~meSOCK_SHUTDOWN;
                }
                else
#endif
                    err = meSocketGetError();
                if(sFp->flags & meSOCK_LOG_ERROR)
                {
                    snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Error: Form data write error %d,%d",ret,err);
                    logFunc(meSOCK_LOG_ERROR,rbuff,logData);
                }
#if MEOPT_OPENSSL
                if(flags & meSOCK_USE_SSL)
                    meSockGetSSLErrors(sFp->flags,rbuff);
#endif
                meSockClose(sFp,1);
                return -23;
            }
            if((fdLen -= ret) <= 0)
                break;
            frmData += ret;
        }
    }
    /* must read header getting now ditch the header, read up to the first blank line */
    if(sFp->flags & meSOCK_LOG_DETAILS)
        logFunc(meSOCK_LOG_DETAILS,(meUByte *) "HTTP Response header:",logData);
    err = 1;
    for(;;)
    {
        meUByte cc;
        if(meSockReadLine(sFp,ioBuff) < 0)
        {
            err = -25;
            break;
        }
        if(sFp->flags & meSOCK_LOG_DETAILS)
            logFunc(meSOCK_LOG_DETAILS,ioBuff,logData);
        if(ioBuff[0] == '\0')
        {
            if((sFp->flags & meSOCK_CHUNKED) && (sFp->length >= 0))
            {
                if(sFp->flags & meSOCK_LOG_ERROR)
                {
                    snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Error: Invalid response, length set for chunked: %d",sFp->length);
                    logFunc(meSOCK_LOG_ERROR,rbuff,logData);
                }
                err = -27;
                break;
            }
            return err;
        }
        if(((cc=(ioBuff[0]|0x20)) == 'c') && ((ioBuff[1]|0x20) == 'o') && ((ioBuff[2]|0x20) == 'n'))
        {
            if(((ioBuff[3]|0x20) == 't') && ((ioBuff[4]|0x20) == 'e') && ((ioBuff[5]|0x20) == 'n') && ((ioBuff[6]|0x20) == 't') && (ioBuff[7] == '-') && ((ioBuff[8]|0x20) == 'l') &&
               ((ioBuff[9]|0x20) == 'e') && ((ioBuff[10]|0x20) == 'n') && ((ioBuff[11]|0x20) == 'g') && ((ioBuff[12]|0x20) == 't') && ((ioBuff[13]|0x20) == 'h') && (ioBuff[14] == ':'))
                sFp->length = atoi((char *) ioBuff+15) ;
            else if(((ioBuff[3]|0x20) == 'n') && ((ioBuff[4]|0x20) == 'e') && ((ioBuff[5]|0x20) == 'c') && ((ioBuff[6]|0x20) == 't') && ((ioBuff[7]|0x20) == 'i') &&
                    ((ioBuff[9]|0x20) == 'o') && ((ioBuff[10]|0x20) == 'n') && (ioBuff[11] == ':'))
            {
                ss = ioBuff+12 ;
                while(((cc=*ss++) == ' ') || (cc == '\t'))
                    ;
                if((cc|0x20) == 'c')
                    /* Connection: close */
                    sFp->flags |= meSOCK_CLOSE;
            }
        }
        else if((cc == 'l') && ((ioBuff[1]|0x20) == 'o') && ((ioBuff[2]|0x20) == 'c') && ((ioBuff[3]|0x20) == 'a') && ((ioBuff[4]|0x20) == 't') &&
                ((ioBuff[5]|0x20) == 'i') && ((ioBuff[6]|0x20) == 'o') && ((ioBuff[7]|0x20) == 'n') && (ioBuff[8] == ':'))
        {
            /* The requested file is not here, its at the given location */
            ss = ioBuff+9 ;
            while(((cc=*ss) == ' ') || (cc == '\t'))
                ss++;
            if(cc != '\0')
            {
                strcpy((char *) rbuff,(char *) ss);
                /* note: carry on reading to validate the rest of the header and get updated cookies etc. */
                err = 0;
            }
        }
        else if((cc == 't') && ((ioBuff[1]|0x20) == 'r') && ((ioBuff[2]|0x20) == 'a') && ((ioBuff[3]|0x20) == 'n') && ((ioBuff[4]|0x20) == 's') &&
                ((ioBuff[5]|0x20) == 'f') && ((ioBuff[6]|0x20) == 'e') && ((ioBuff[7]|0x20) == 'r') && (ioBuff[8] == '-') && ((ioBuff[9]|0x20) == 'e') &&
                ((ioBuff[10]|0x20) == 'n') && ((ioBuff[11]|0x20) == 'c') && ((ioBuff[12]|0x20) == 'o') && ((ioBuff[13]|0x20) == 'd') && ((ioBuff[14]|0x20) == 'i') &&
                ((ioBuff[15]|0x20) == 'n') && ((ioBuff[16]|0x20) == 'g') && (ioBuff[17] == ':'))
        {
            ss = ioBuff+18;
            while(((cc=*ss++) == ' ') || (cc == '\t'))
                ;
            if((cc == 'c') && ((ss[0]|0x20) == 'h') && ((ss[1]|0x20) == 'u') && ((ss[2]|0x20) == 'n') && ((ss[3]|0x20) == 'k') && ((ss[4]|0x20) == 'e') && ((ss[5]|0x20) == 'd'))
            {
                sFp->flags |= meSOCK_CHUNKED;
                sFp->chnkLen = -1;
                ss += 6;
                while(((cc=*ss++) == ' ') || (cc == '\t'))
                    ;
            }
            if(cc != '\0')
            {
                if(sFp->flags & meSOCK_LOG_ERROR)
                {
                    snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Error: Unsupported transfer encoding: %s",ss-1);
                    logFunc(meSOCK_LOG_ERROR,rbuff,logData);
                }
                err = -26;
                break;
            }
        }
        else if((cookie != NULL) && (cc == 's') && ((ioBuff[1]|0x20) == 'e') && ((ioBuff[2]|0x20) == 't') && (ioBuff[3] == '-') && ((ioBuff[4]|0x20) == 'c') &&
                ((ioBuff[5]|0x20) == 'o') && ((ioBuff[6]|0x20) == 'o') && ((ioBuff[7]|0x20) == 'k') && ((ioBuff[8]|0x20) == 'i') && ((ioBuff[9]|0x20) == 'e') && (ioBuff[10] == ':'))
        {
            meUByte *vv, *ee, *dd, *de;
            ss = ioBuff+11;
            while(((cc=*ss) == ' ') || (cc == '\t'))
                ss++;
            if((cc != '\0') && (cc != '=') && ((vv = (meUByte *) strchr((char *) ss,'=')) != NULL) && ((ee = (meUByte *) strchr((char *) vv,';')) != NULL))
            {
                vv++;
                *ee = '\0';
                if((cookie->value != NULL) && (cookie->value[0] != '\0'))
                {
                    ll = vv - ss;
                    if(strncmp((char *) cookie->value,(char *) ss,ll))
                    {
                        cc = *vv;
                        *vv = '\0';
                        ss[-2] = ';';
                        ss[-1] = ' ';
                        if((dd=(meUByte *) strstr((char *) cookie->value,(char *) (ss-2))) != NULL)
                        {
                            if((de = (meUByte *) strchr((char *) (dd+ll+2),';')) != NULL)
                            {
                                while((*dd++ = *de++) != '\0')
                                    ;
                            }
                            else
                                *dd = '\0';
                        }
                        *vv = cc;
                        de = cookie->value;
                    }
                    else if((de = (meUByte *) strchr((char *) (cookie->value+ll),';')) != NULL)
                        de += 2;
                }
                else
                    de = NULL;
                if(*vv != '\0')
                {
                    meUByte *ov=NULL;
                    ll = strlen((char *) ss)+1;
                    if(de != NULL)
                        ll += strlen((char *) de)+2;
                    if(ll > cookie->buffLen)
                    {
                        ov = cookie->value;
                        ll = ((ll + 256) & ~15);
                        if((cookie->value = (meUByte *) malloc(ll)) == NULL)
                        {
                            if(sFp->flags & meSOCK_LOG_WARNING)
                                logFunc(meSOCK_LOG_WARNING,(meUByte *) "meSock Error: Cookie malloc failure",logData);
                            ll = 0;
                        }
                        cookie->buffLen = ll;
                    }
                    if((dd = cookie->value) != NULL)
                    {
                        if(de != NULL)
                        {
                            if(de == dd)
                            {
                                while(*dd++ != '\0')
                                    ;
                            }
                            else
                            {
                                while((*dd++ = *de++) != '\0')
                                    ;
                            }
                            dd[-1] = ';';
                            *dd++ = ' ';
                        }
                        while((*dd++ = *ss++) != '\0')
                            ;
                    }
                    if(ov != NULL)
                        free(ov);
                }
                else if(de == NULL)
                    cookie->value[0] = '\0';
                else if(de != cookie->value)
                {
                    dd = cookie->value;
                    while((*dd++ = *de++) != '\0')
                        ;
                }
            }
        }
    }
    meSockClose(sFp,1);
    sFp->length = 0;
    return err;
}

static meInt
meSockControlReadLine(meSockFile *sFp, meUByte *buff)
{
    meUByte *dd=buff, cc ;
    meInt ret, err, ll=meSOCK_BUFF_SIZE;

    for(;;)
    {
#if MEOPT_OPENSSL
        if(sFp->flags & (meSOCK_USE_SSL|meSOCK_EXPLICIT_SSL))
            ret = OPENSSLFunc(SSL_read)(sFp->ctrlSsl,dd,1);
        else
#endif
            ret = meSocketRead(sFp->ctrlSck,(char *) dd,1,0);
        if(ret <= 0)
        {
#if MEOPT_OPENSSL
            if(sFp->flags & meSOCK_USE_SSL)
            {
                if(((err = OPENSSLFunc(SSL_get_error)(sFp->ctrlSsl,ret)) == SSL_ERROR_SYSCALL) || (err == SSL_ERROR_SSL))
                    sFp->flags &= ~meSOCK_CTRL_SHUTDOWN;
            }
            else
#endif
                err = (ret == 0) ? 0:meSocketGetError();
            if(sFp->flags & meSOCK_LOG_ERROR)
            {
                snprintf((char *) buff,meSOCK_BUFF_SIZE,"meSock Error: Failed to read FTP control reply - %d,%d",ret,err);
                logFunc(meSOCK_LOG_ERROR,buff,logData);
            }
            sFp->flags |= meSOCK_CTRL_NO_QUIT;
            meSockClose(sFp,1);
            return -1;
        }
        if((cc=*dd) == '\n')
            break ;
        if((cc != '\r') && (--ll > 0))
            dd++;
    }
    *dd = '\0';
    if(sFp->flags & meSOCK_LOG_VERBOSE)
        logFunc(meSOCK_LOG_VERBOSE,buff,logData);
    return meSOCK_BUFF_SIZE-ll;
}

meInt
meSockFtpReadReply(meSockFile *sFp, meUByte *buff)
{
    meInt ret ;

    if(sFp->flags & meSOCK_LOG_VERBOSE)
        logFunc(meSOCK_LOG_VERBOSE,(meUByte *) "FTP Reply:",logData);
    if(meSockControlReadLine(sFp,buff) < 4)
        return ftpERROR;

    ret = buff[0] - '0' ;
    if(buff[3] == '-')
    {
        /* multi-line reply */
        meUByte c0,c1,c2 ;
        c0 = buff[0] ;
        c1 = buff[1] ;
        c2 = buff[2] ;
        do {
            if(meSockControlReadLine(sFp,buff) <= 0)
                return ftpERROR;
        } while((buff[0] != c0) || (buff[1] != c1) || (buff[2] != c2) || (buff[3] != ' ')) ;
    }
    if((ret < ftpPOS_PRELIMIN) || (ret > ftpPOS_INTERMED))
        return ftpERROR;
    return ret;
}

meInt
meSockFtpCommand(meSockFile *sFp, meUByte *rbuff, char *fmt, ...)
{
    va_list ap;
    meInt ii, ret;

    va_start(ap,fmt);
    ii = vsnprintf((char *) rbuff,meSOCK_BUFF_SIZE,fmt,ap);
    va_end(ap);
    if(sFp->flags & (meSOCK_LOG_DETAILS|meSOCK_LOG_VERBOSE))
    {
        if(!strncmp((char *) rbuff,"PASS",4))
            logFunc(meSOCK_LOG_DETAILS,(meUByte *) "PASS XXXX",logData);
        else
            logFunc(meSOCK_LOG_DETAILS,rbuff,logData);
    }
    rbuff[ii++] = '\r' ;
    rbuff[ii++] = '\n' ;
    rbuff[ii]   = '\0' ;
#if MEOPT_OPENSSL
    if(sFp->flags & (meSOCK_USE_SSL|meSOCK_EXPLICIT_SSL))
        ret = OPENSSLFunc(SSL_write)(sFp->ctrlSsl,rbuff,ii);
    else
#endif
        ret = meSocketWrite(sFp->ctrlSck,(char *) rbuff,ii,0);
    if(ret <= 0)
    {
#if MEOPT_OPENSSL
        if(sFp->flags & meSOCK_USE_SSL)
        {
            if(((ii=OPENSSLFunc(SSL_get_error)(sFp->ctrlSsl,ret)) == SSL_ERROR_SYSCALL) || (ii == SSL_ERROR_SSL))
                sFp->flags &= ~meSOCK_CTRL_SHUTDOWN;
        }
        else
#endif
            ii = meSocketGetError();
        if(sFp->flags & meSOCK_LOG_ERROR)
        {
            snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Error: FTP command write error %d,%d",ret,ii);
            logFunc(meSOCK_LOG_ERROR,rbuff,logData);
        }
#if MEOPT_OPENSSL
        if(sFp->flags & meSOCK_USE_SSL)
            meSockGetSSLErrors(sFp->flags,rbuff);
#endif
        sFp->flags |= meSOCK_CTRL_NO_QUIT;
        meSockClose(sFp,1);
        return -43;
    }
    return meSockFtpReadReply(sFp,rbuff);
}

meInt
meSockFtpGetDataChannel(meSockFile *sFp, meUByte *buff)
{
    struct sockaddr_in sa;
    int aai[4], ppi[2];
    meUByte *ss;

    if(((sFp->flags & meSOCK_INUSE) != 0) || (sFp->length != -2))
    {
        if(sFp->flags & meSOCK_LOG_ERROR)
            logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: FTP data channel already created",logData);
        return ftpERROR;
    }

    sFp->flags |= meSOCK_INUSE;
    if(meSockFtpCommand(sFp,buff,"PASV") != ftpPOS_COMPLETE)
        return ftpERROR;

    if(((ss=(meUByte *) strchr((char *) buff,'(')) == NULL) || (sscanf((char *) ss,"(%d,%d,%d,%d,%d,%d)",aai,aai+1,aai+2,aai+3,ppi,ppi+1) != 6))
    {
        if(sFp->flags & meSOCK_LOG_ERROR)
            logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: Failed to extract PASSIVE address",logData);
        return ftpERROR;
    }
    memset(&sa,0,sizeof(sa));
    sa.sin_family = AF_INET;
    ss = (meUByte *)&(sa.sin_addr);
    ss[0] = (meUByte) aai[0] ;
    ss[1] = (meUByte) aai[1] ;
    ss[2] = (meUByte) aai[2] ;
    ss[3] = (meUByte) aai[3] ;
    ss = (meUByte *)&(sa.sin_port);
    ss[0] = (meUByte) ppi[0] ;
    ss[1] = (meUByte) ppi[1] ;

    /* only open and connect the plain socket at this stage, all SSL setup must be done after the sending of the command */
    if((sFp->sck = meSocketOpen(AF_INET,SOCK_STREAM,IPPROTO_TCP)) == meBadSocket)
    {
        if(sFp->flags & meSOCK_LOG_ERROR)
        {
            sprintf((char *) buff,"meSock Error: Failed to open data channel - Error %d",meSocketGetError()) ;
            logFunc(meSOCK_LOG_ERROR,buff,logData);
        }
        return ftpERROR;
    }
    if(meSocketConnect(sFp->sck,(struct sockaddr*) &sa,sizeof(sa)) != 0)
    {
        if(sFp->flags & meSOCK_LOG_ERROR)
        {
            sprintf((char *) buff,"meSock Error: Failed to connect data channel - Error %d",meSocketGetError()) ;
            logFunc(meSOCK_LOG_ERROR,buff,logData);
        }
        return ftpERROR;
    }
    return 1;
}


meInt
meSockFtpConnectData(meSockFile *sFp, meUByte *buff)
{
    if(((sFp->flags & meSOCK_INUSE) == 0) || (sFp->length != -2))
    {
        if(sFp->flags & meSOCK_LOG_ERROR)
            logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: Data channel not created",logData);
        return ftpERROR;
    }
#if MEOPT_OPENSSL
    if(sFp->flags & meSOCK_USE_SSL)
    {
        SSL_SESSION *ssn;
        meInt ret,err;
        if((sFp->ssl=OPENSSLFunc(SSL_new)(meSockCtx)) == NULL)
        {
            if(sFp->flags & meSOCK_LOG_ERROR)
                logFunc(meSOCK_LOG_VERBOSE,(meUByte *) "meSock Error: Failed to create new data SSL",logData);
            meSockClose(sFp,1);
            return -12;
        }
        /*Bind the data socket to the SSL structure */
        if((ret = OPENSSLFunc(SSL_set_fd)(sFp->ssl,sFp->sck)) <= 0)
        {
            err = OPENSSLFunc(SSL_get_error)(sFp->ssl,ret);
            if(sFp->flags & meSOCK_LOG_ERROR)
            {
                snprintf((char *) buff,meSOCK_BUFF_SIZE,"meSock Error: Failed to set data SSL fd - error %d,%d",ret,err);
                logFunc(meSOCK_LOG_ERROR,buff,logData);
            }
            meSockGetSSLErrors(sFp->flags,buff);
            meSockClose(sFp,1);
            return -15;
        }
        /* Get the SSL session from the control to setup the data connection */
        if((ssn = OPENSSLFunc(SSL_get_session)(sFp->ctrlSsl)) == NULL)
        {
            err = OPENSSLFunc(SSL_get_error)(sFp->ssl,ret);
            if(sFp->flags & meSOCK_LOG_ERROR)
                logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: Failed to get control SSL session",logData);
            meSockGetSSLErrors(sFp->flags,buff);
            meSockClose(sFp,1);
            return -15;
        }
        if((ret = OPENSSLFunc(SSL_set_session)(sFp->ssl,ssn)) <= 0)
        {
            err = OPENSSLFunc(SSL_get_error)(sFp->ssl,ret);
            if(sFp->flags & meSOCK_LOG_ERROR)
            {
                snprintf((char *) buff,meSOCK_BUFF_SIZE,"meSock Error: Failed to set data SSL session - error %d,%d",ret,err);
                logFunc(meSOCK_LOG_ERROR,buff,logData);
            }
            meSockGetSSLErrors(sFp->flags,buff);
            meSockClose(sFp,1);
            return -15;
        }
        /* Connect data to the server, SSL layer. */
        if((ret=OPENSSLFunc(SSL_connect)(sFp->ssl)) <= 0)
        {
            err = OPENSSLFunc(SSL_get_error)(sFp->ssl,ret);
            if(sFp->flags & meSOCK_LOG_ERROR)
            {
                snprintf((char *) buff,meSOCK_BUFF_SIZE,"meSock Error: Failed to connect data SSL - error %d,%d",ret,err);
                logFunc(meSOCK_LOG_ERROR,buff,logData);
            }
            meSockGetSSLErrors(sFp->flags,buff);
            meSockClose(sFp,1);
            return -17;
        }
        sFp->flags |= meSOCK_SHUTDOWN;
    }
#endif
    sFp->length = -1;
    return 1;
}

meInt
meSockFtpOpen(meSockFile *sFp, meUShort flags, meUByte *host, meInt port, meUByte *user, meUByte *pass, meUByte *rbuff)
{
    meInt ret, err;
    meSOCKET sck;
#if MEOPT_OPENSSL
    SSL *ssl;
#endif

    if(logFunc == NULL)
        flags &= ~(meSOCK_LOG_STATUS|meSOCK_LOG_WARNING|meSOCK_LOG_DETAILS|meSOCK_LOG_VERBOSE);
    flags = (flags & meSOCK_PUBLIC_MASK) | meSOCK_CTRL_INUSE|meSOCK_CLOSE;
    if(!port)
        port = (flags & meSOCK_USE_SSL) ? 990:21;

    if(meSockIsInUse(sFp) && ((sFp->cprt != port) || (sFp->length != -2) || ((sck=sFp->ctrlSck) == meBadSocket) ||
                              ((sFp->flags & (meSOCK_INUSE|meSOCK_CTRL_INUSE)) != meSOCK_CTRL_INUSE) ||
                              (!(flags & meSOCK_EXPLICIT_SSL) && ((sFp->flags & meSOCK_USE_SSL) != (flags & meSOCK_USE_SSL))) ||
                              strcmp((char *) sFp->chst,(char *) host) || strcmp((char *) sFp->cusr,((user == NULL) ? "":(char *) user))))
        meSockClose(sFp,1);
    sFp->length = -2;
    sFp->chnkLen = 0;
    if(meSockIsInUse(sFp))
    {
        sFp->flags = (sFp->flags & (meSOCK_USE_SSL|meSOCK_EXPLICIT_SSL|meSOCK_CTRL_SHUTDOWN)) | (flags & ~(meSOCK_USE_SSL|meSOCK_EXPLICIT_SSL));
#if MEOPT_OPENSSL
        ssl = sFp->ctrlSsl;
#else
    }
    else if(flags & (meSOCK_USE_SSL|meSOCK_EXPLICIT_SSL))
    {
        if(sFp->flags & meSOCK_LOG_ERROR)
            logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: FTPS not supported in this build",logData);
        return -24;
#endif
    }
    else
    {
        struct sockaddr_in sa;
        struct hostent *hp;

        sFp->home = NULL;
        sFp->sck = meBadSocket;
        sFp->ctrlSck = meBadSocket;
#if MEOPT_OPENSSL
        sFp->ssl = NULL;
        sFp->ctrlSsl = NULL;
#endif
        sFp->cprt = -1;
        if(ioBuffSz < meSOCK_IOBUF_MIN)
        {
            if(flags & meSOCK_LOG_ERROR)
                logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: Setup not called with IO buffer",logData);
            return -10;
        }
#if MEOPT_OPENSSL
        if(flags & (meSOCK_USE_SSL|meSOCK_EXPLICIT_SSL))
        {
            if((meSockCtx == NULL) && ((ret = meSockInit(flags,rbuff)) < 0))
                return ret;
            OPENSSLFunc(ERR_clear_error)();
        }
#endif
        if(flags & meSOCK_LOG_STATUS)
        {
            snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"Connecting to %s:%d",host,port);
            logFunc(meSOCK_LOG_STATUS,rbuff,logData);
        }
        if(((hp = gethostbyname((char *) host)) == NULL) ||
           (hp->h_length > ((int) sizeof(struct in_addr))) ||
           (hp->h_addrtype != AF_INET))
        {
            if(flags & meSOCK_LOG_ERROR)
            {
                snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Error: Failed to get host [%s] - error %d",host,(hp == NULL) ? meSocketGetError():-1);
                logFunc(meSOCK_LOG_ERROR,rbuff,logData);
            }
            return -11;
        }
        memset(&sa,0,sizeof(sa));
        sa.sin_family = AF_INET;
        memcpy(&sa.sin_addr.s_addr,hp->h_addr,hp->h_length);
        sa.sin_port = htons(port);

#if MEOPT_OPENSSL
        if(flags & meSOCK_USE_SSL)
        {
            if((ssl=OPENSSLFunc(SSL_new)(meSockCtx)) == NULL)
            {
                if(flags & meSOCK_LOG_ERROR)
                    logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: Failed to created new control SSL",logData);
                return -12;
            }
            sFp->ctrlSsl = ssl;

            meSockHostnameGetSni((char *) host,(char *) ioBuff);
            if(!meSockIsValidIpAddress((char *) ioBuff))
            {
                // SSL_set_tlsext_host_name(ssl,ioBuff);
                long rc = OPENSSLFunc(SSL_ctrl)(ssl,SSL_CTRL_SET_TLSEXT_HOSTNAME,TLSEXT_NAMETYPE_host_name,(void *) ioBuff);
                if(rc == 0)
                {
                    OPENSSLFunc(SSL_free)(ssl);
                    if(flags & meSOCK_LOG_ERROR)
                        logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: Failed to set control TLS server-name indication",logData);
                    return -13;
                }
            }
        }
#endif
        if((sck=meSocketOpen(AF_INET,SOCK_STREAM,IPPROTO_TCP)) == meBadSocket)
        {
            if(flags & meSOCK_LOG_ERROR)
                logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: Failed to create control socket",logData);
#if MEOPT_OPENSSL
            if(flags & meSOCK_USE_SSL)
                OPENSSLFunc(SSL_free)(ssl);
#endif
            return -14;
        }
        if(timeoutSnd > 0)
        {
            /* using setsockopt to set the send timeout to given millisecs, alternatively use select(0,fd_set,NULL,NULL,struct timeval) */
            struct timeval to;
            to.tv_sec = timeoutSnd;
            to.tv_usec = 0;
            if((ret=setsockopt(sck,SOL_SOCKET,SO_SNDTIMEO,(char *) &to,sizeof(to))) < 0)
            {
                if(flags & meSOCK_LOG_WARNING)
                {
                    snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Warning: Failed to set send timeout - error %d,%d",ret,meSocketGetError());
                    logFunc(meSOCK_LOG_WARNING,rbuff,logData);
                }
            }
        }
        if(timeoutRcv > 0)
        {
            struct timeval to;
            to.tv_sec = timeoutRcv;
            to.tv_usec = 0;
            if((ret=setsockopt(sck,SOL_SOCKET,SO_RCVTIMEO,(char *) &to,sizeof(to))) < 0)
            {
                if(flags & meSOCK_LOG_WARNING)
                {
                    snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Warning: Failed to set recv timeout - error %d,%d",ret,meSocketGetError());
                    logFunc(meSOCK_LOG_WARNING,rbuff,logData);
                }
            }
        }
        sFp->ctrlSck = sck;
        sFp->flags = (flags & ~meSOCK_EXPLICIT_SSL);

#if MEOPT_OPENSSL
        /*Bind the socket to the SSL structure*/
        if((flags & meSOCK_USE_SSL) && ((ret = OPENSSLFunc(SSL_set_fd)(ssl,sck)) < 1))
        {
            err = OPENSSLFunc(SSL_get_error)(ssl,ret);
            if(flags & meSOCK_LOG_ERROR)
            {
                snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Error: Failed to set control SSL fd - error %d,%d",ret,err);
                logFunc(meSOCK_LOG_ERROR,rbuff,logData);
            }
            meSockGetSSLErrors(flags,rbuff);
            meSockClose(sFp,1);
            return -15;
        }
#endif
        /* Connect to the server, TCP/IP layer,*/
        if((err=meSocketConnect(sck,(struct sockaddr*) &sa,sizeof(sa))) != 0)
        {
            if(sFp->flags & meSOCK_LOG_ERROR)
            {
                snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Error: Failed to connect control socket - error #%d",meSocketGetError());
                logFunc(meSOCK_LOG_ERROR,rbuff,logData);
            }
            meSockClose(sFp,1);
            return -16;
        }

#if MEOPT_OPENSSL
        if(flags & meSOCK_USE_SSL)
        {
            /*Connect to the server, SSL layer.*/
            if((ret=OPENSSLFunc(SSL_connect)(ssl)) < 1)
            {
                err = OPENSSLFunc(SSL_get_error)(ssl,ret);
                if(sFp->flags & meSOCK_LOG_ERROR)
                {
                    snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Error: Failed to connect control SSL - error %d,%d",ret,err);
                    logFunc(meSOCK_LOG_ERROR,rbuff,logData);
                }
                meSockGetSSLErrors(sFp->flags,rbuff);
                meSockClose(sFp,1);
                return -17;
            }

            /*Print out connection details*/
            if(flags & meSOCK_LOG_DETAILS)
            {
                snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"FTP control SSL connected on socket %x, version: %s, cipher: %s",sck,OPENSSLFunc(SSL_get_version)(ssl),
                         OPENSSLFunc(SSL_CIPHER_get_name)(OPENSSLFunc(SSL_get_current_cipher)(ssl)));
                logFunc(meSOCK_LOG_DETAILS,rbuff,logData);
            }
            if(!meSockCheckCertificate(sFp,(char *) ioBuff,rbuff))
            {
                meSockClose(sFp,1);
                return -18;
            }
            sFp->flags |= meSOCK_CTRL_SHUTDOWN;
        }
        else
#endif
            if(flags & meSOCK_LOG_DETAILS)
        {
            snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"FTP control connected on socket %x",sck);
            logFunc(meSOCK_LOG_DETAILS,rbuff,logData);
        }
        /* get the initial message */
        if(meSockFtpReadReply(sFp,rbuff) != ftpPOS_COMPLETE)
        {
            meSockClose(sFp,1);
            return -31;
        }
        if((flags & (meSOCK_USE_SSL|meSOCK_EXPLICIT_SSL)) == meSOCK_EXPLICIT_SSL)
        {
#if MEOPT_OPENSSL
            if(meSockFtpCommand(sFp,rbuff,"AUTH TLS") == ftpPOS_COMPLETE)
            {
                if((ssl=OPENSSLFunc(SSL_new)(meSockCtx)) == NULL)
                {
                    if(sFp->flags & meSOCK_LOG_ERROR)
                        logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: Failed to create new control SSL",logData);
                    return -12;
                }
                sFp->ctrlSsl = ssl;

                meSockHostnameGetSni((char *) host,(char *) ioBuff);
                if(!meSockIsValidIpAddress((char *) ioBuff))
                {
                    // SSL_set_tlsext_host_name(ssl,ioBuff);
                    long rc = OPENSSLFunc(SSL_ctrl)(ssl,SSL_CTRL_SET_TLSEXT_HOSTNAME,TLSEXT_NAMETYPE_host_name,(void *) ioBuff);
                    if(rc == 0)
                    {
                        OPENSSLFunc(SSL_free)(ssl);
                        if(sFp->flags & meSOCK_LOG_ERROR)
                            logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: Failed to set control TLS server-name indication",logData);
                        return -13;
                    }
                }
                /*Bind the socket to the SSL structure*/
                if((ret = OPENSSLFunc(SSL_set_fd)(ssl,sck)) < 1)
                {
                    err = OPENSSLFunc(SSL_get_error)(ssl,ret);
                    if(sFp->flags & meSOCK_LOG_ERROR)
                    {
                        snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Error: Failed to set control SSL fd - error %d,%d",ret,err);
                        logFunc(meSOCK_LOG_ERROR,rbuff,logData);
                    }
                    meSockGetSSLErrors(sFp->flags,rbuff);
                    meSockClose(sFp,1);
                    return -15;
                }
                if((ret=OPENSSLFunc(SSL_connect)(ssl)) < 1)
                {
                    err = OPENSSLFunc(SSL_get_error)(ssl,ret);
                    if(sFp->flags & meSOCK_LOG_ERROR)
                    {
                        snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"meSock Error: Failed to connect control SSL - error %d,%d",ret,err);
                        logFunc(meSOCK_LOG_ERROR,rbuff,logData);
                    }
                    meSockGetSSLErrors(sFp->flags,rbuff);
                    meSockClose(sFp,1);
                    return -17;
                }
                /*Print out connection details*/
                if(sFp->flags & meSOCK_LOG_DETAILS)
                {
                    snprintf((char *) rbuff,meSOCK_BUFF_SIZE,"Control SSL connected on socket %x, version: %s, cipher: %s",sck,OPENSSLFunc(SSL_get_version)(ssl),
                             OPENSSLFunc(SSL_CIPHER_get_name)(OPENSSLFunc(SSL_get_current_cipher)(ssl)));
                    logFunc(meSOCK_LOG_DETAILS,rbuff,logData);
                }
                if(!meSockCheckCertificate(sFp,(char *) ioBuff,rbuff))
                {
                    meSockClose(sFp,1);
                    return -18;
                }
                sFp->flags |= meSOCK_EXPLICIT_SSL|meSOCK_CTRL_SHUTDOWN;
                meSockFtpCommand(sFp,rbuff,"PBSZ 0");
                if(meSockFtpCommand(sFp,rbuff,"PROT P") == ftpPOS_COMPLETE)
                    sFp->flags |= meSOCK_USE_SSL;
                else if(logFunc != NULL)
                    /* Always report this warning as this could be a security issue */
                    logFunc(meSOCK_LOG_WARNING,(meUByte *) "meSockFtp Warning: Server refused data protection",logData);
            }
            else
#endif
            {
#if MEOPT_OPENSSL
                if(logFunc != NULL)
                    /* Always report this warning as this could be a security issue */
                    logFunc(meSOCK_LOG_WARNING,(meUByte *) "meSock Warning: Server does not support explicit TLS",logData);
#else
                if(sFp->flags & meSOCK_LOG_ERROR)
                    logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: FTP SSL not supported in this build",logData);
#endif
                sFp->flags &= ~meSOCK_EXPLICIT_SSL;
            }
        }
        if((ret=meSockFtpCommand(sFp,rbuff,"USER %s",(user == NULL) ? "anonymous":(char *) user)) == ftpPOS_INTERMED)
        {
            if(meSockFtpCommand(sFp,rbuff,"PASS %s",(pass == NULL) ? "ftp@example.com":(char *) pass) != ftpPOS_COMPLETE)
            {
                if(sFp->flags & meSOCK_LOG_ERROR)
                    logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: Failed to login",logData);
                meSockClose(sFp,1);
                return -33;
            }
        }
        else if(ret != ftpPOS_COMPLETE)
        {
            if(sFp->flags & meSOCK_LOG_ERROR)
                logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: Failed to send USER command",logData);
            meSockClose(sFp,1);
            return -32;
        }
        else if((pass != NULL) && (logFunc != NULL))
            /* Always report this warning as it could be a security breach */
            logFunc(meSOCK_LOG_WARNING,(meUByte *) "meSock Warning: User password not required",logData);

        if(meSockFtpCommand(sFp,rbuff,"TYPE I") != ftpPOS_COMPLETE)
        {
            if(sFp->flags & meSOCK_LOG_ERROR)
                logFunc(meSOCK_LOG_ERROR,(meUByte *) "meSock Error: Failed to make connection binary",logData);
            meSockClose(sFp,1);
            return -34;
        }
        sFp->cprt = port;
        strcpy((char *) sFp->chst,(char *) host);
        if(user != NULL)
            strcpy((char *) sFp->cusr,(char *) user);
        else
            sFp->cusr[0] = '\0';
        if(meSockFtpCommand(sFp,rbuff,"PWD") == ftpPOS_COMPLETE)
        {
            char *hs, *he;
            if(((hs = strchr((char *) rbuff,'"')) != NULL) && ((he = strchr(++hs,'"')) != NULL))
            {
                if(he[-1] == '/')
                    he[-1] = '\0';
                else
                    he[0] =  '\0';
                if(hs[0] != '\0')
                    sFp->home = (meUByte *) strdup(hs);
            }
        }
    }
    return 1;
}


int
meSockRead(meSockFile *sFp, int sz, meUByte *buff, int offs)
{
    meInt ll, err=0, ret;

    if((ll=sFp->chnkLen))
    {
        if(ll == -1)
        {
            meInt ii=0;
            meUByte ba[4], bb;
            ll = 0;
            for(;;)
            {
#if MEOPT_OPENSSL
                if(sFp->flags & meSOCK_USE_SSL)
                    ret = OPENSSLFunc(SSL_read)(sFp->ssl,ba,1);
                else
#endif
                    ret = meSocketRead(sFp->sck,(char *) ba,1,0);
                if(ret <= 0)
                {
#if MEOPT_OPENSSL
                    if(sFp->flags & meSOCK_USE_SSL)
                        err = OPENSSLFunc(SSL_get_error)(sFp->ssl,ret);
                    else
#endif
                        err = (ret == 0) ? 0:meSocketGetError();
                    ll = -3;
                    break;
                }
                if((bb = ba[0]) == '\n')
                {
                    if(ii == 0)
                        ll = -6;
                    break;
                }
                else if(ii >= 0)
                {
                    if((bb == ';') || (bb == '\r'))
                    {
                        if(ii == 0)
                        {
                            ll = -5;
                            break;
                        }
                        ii = -2;
                    }
                    else if((bb >= '0') && (bb <= '9'))
                        ll = (ll << 4) | (bb - '0');
                    else if((bb >= 'A') && (bb <= 'F'))
                        ll = (ll << 4) | (bb - 'A' + 10);
                    else if((bb >= 'a') && (bb <= 'f'))
                        ll = (ll << 4) | (bb - 'a' + 10);
                    else
                    {
                        ll = -4;
                        break;
                    }
                    if(++ii > 7)
                    {
                        ll = -7;
                        break;
                    }
                }
            }
        }
        if(ll > 0)
        {
            if(sz > ll)
                sz = ll;
#if MEOPT_OPENSSL
            if(sFp->flags & meSOCK_USE_SSL)
                ret = OPENSSLFunc(SSL_read)(sFp->ssl,buff+offs,sz);
            else
#endif
                ret = meSocketRead(sFp->sck,(char *) (buff+offs),sz,0);
            if(ret <= 0)
            {
#if MEOPT_OPENSSL
                if(sFp->flags & meSOCK_USE_SSL)
                    err = OPENSSLFunc(SSL_get_error)(sFp->ssl,ret);
                else
#endif
                    err = (ret == 0) ? 0:meSocketGetError();
                ret = -1;
            }
            else if(ret < ll)
            {
                sFp->chnkLen = ll-ret;
                return ret;
            }
        }
        else
            ret = ll;

        if(ret >= 0)
        {
            /* must read the chunk terminator */
            meUByte ba[4];
#if MEOPT_OPENSSL
            if(sFp->flags & meSOCK_USE_SSL)
                ll = OPENSSLFunc(SSL_read)(sFp->ssl,ba,2);
            else
#endif
                ll = meSocketRead(sFp->sck,(char *) ba,2,0);
            if(ll <= 0)
            {
#if MEOPT_OPENSSL
                if(sFp->flags & meSOCK_USE_SSL)
                    err = OPENSSLFunc(SSL_get_error)(sFp->ssl,ret);
                else
#endif
                    err = (ret == 0) ? 0:meSocketGetError();
                ret = -8;
            }
            else if((ll != 2) || (ba[0] != '\r') || (ba[1] != '\n'))
                ret = -9;
            else if(ret)
            {
                sFp->chnkLen = -1;
                return ret;
            }
            else
            {
                if(sFp->flags & meSOCK_LOG_VERBOSE)
                    logFunc(meSOCK_LOG_VERBOSE,(meUByte *) "meSock Verbose: Got data stream end",logData);
                sFp->length = 0;
                sFp->chnkLen = -2;
                return 0;
            }
        }
    }
#if MEOPT_OPENSSL
    else if(sFp->flags & meSOCK_USE_SSL)
    {
        if((ret = OPENSSLFunc(SSL_read)(sFp->ssl,buff+offs,sz)) > 0)
            return ret;
        err = OPENSSLFunc(SSL_get_error)(sFp->ssl,ret);
        if((sFp->length == -1) && (err == SSL_ERROR_ZERO_RETURN))
        {
            /* Conent-lenth not returned and we've likely hit the correct end of the data - return 0 */
            if(sFp->flags & meSOCK_LOG_VERBOSE)
                logFunc(meSOCK_LOG_VERBOSE,(meUByte *) "meSock Verbose: Got data stream end",logData);
            sFp->length = -2;
            return 0;
        }
    }
#endif
    else
    {
        if((ret = meSocketRead(sFp->sck,(char *) (buff+offs),sz,0)) > 0)
            return ret;
        if(ret < 0)
            err = meSocketGetError();
        else if(sFp->length == -1)
        {
            /* Conent-lenth not returned and we've likely hit the correct end of the data - return 0 */
            if(sFp->flags & meSOCK_LOG_VERBOSE)
                logFunc(meSOCK_LOG_VERBOSE,(meUByte *) "meSock Verbose: Got data stream end",logData);
            sFp->length = -2;
            return 0;
        }
        else
            err = 0;
    }
    if(sFp->flags & meSOCK_LOG_ERROR)
    {
        snprintf((char *) buff,meSOCK_BUFF_SIZE,"meSock Error: Read failure - %d,%d,%d",ll,ret,err);
        logFunc(meSOCK_LOG_ERROR,buff,logData);
    }
#if MEOPT_OPENSSL
    if(sFp->flags & meSOCK_USE_SSL)
    {
        if((err == SSL_ERROR_SYSCALL) || (err == SSL_ERROR_SSL))
            sFp->flags &= ~meSOCK_SHUTDOWN;
        meSockGetSSLErrors(sFp->flags,buff);
    }
#endif
    sFp->flags |= meSOCK_CLOSE;
    return -1;
}

int
meSockWrite(meSockFile *sFp, int sz, meUByte *buff, int offs)
{
    int err,ret;

#if MEOPT_OPENSSL
    if(sFp->flags & meSOCK_USE_SSL)
    {
        for(;;)
        {
            if((ret = OPENSSLFunc(SSL_write)(sFp->ssl,buff+offs,sz)) <= 0)
                break;
            if((sz -= ret) <= 0)
                return 1;
            offs += ret;
        }
        if(((err=OPENSSLFunc(SSL_get_error)(sFp->ssl,ret)) == SSL_ERROR_SYSCALL) || (err == SSL_ERROR_SSL))
            sFp->flags &= ~meSOCK_SHUTDOWN;
    }
    else
#endif
    {
        for(;;)
        {
            if((ret = meSocketWrite(sFp->sck,(char *) (buff+offs),sz,0)) <= 0)
                break;
            if((sz -= ret) <= 0)
                return 1;
            offs += ret;
        }
        err = meSocketGetError();
    }
    if(sFp->flags & meSOCK_LOG_ERROR)
    {
        snprintf((char *) buff,meSOCK_BUFF_SIZE,"meSock Error: Write failure - %d,%d",ret,err);
        logFunc(meSOCK_LOG_ERROR,buff,logData);
    }
#if MEOPT_OPENSSL
    if(sFp->flags & meSOCK_USE_SSL)
        meSockGetSSLErrors(sFp->flags,buff);
#endif
    sFp->flags |= meSOCK_CLOSE;
    return -1;
}

int
meSockClose(meSockFile *sFp, int force)
{
    if(meSockIsInUse(sFp))
    {
        if(!force && meSockFileCanReuse(sFp))
            return 1;
#if MEOPT_OPENSSL
        if(sFp->ssl != NULL)
        {
            if(sFp->flags & meSOCK_SHUTDOWN)
            {
                int err = OPENSSLFunc(SSL_shutdown)(sFp->ssl);
                if(err < 0)
                {
                    if(sFp->flags & meSOCK_LOG_WARNING)
                        logFunc(meSOCK_LOG_WARNING,(meUByte *) "meSock Warning: SSL shutdown failed",logData);
                }
                /* do not need to call SSL_shutdown again if err == 0 as we are closing the socket */
                else if(sFp->flags & meSOCK_LOG_VERBOSE)
                    logFunc(meSOCK_LOG_VERBOSE,(meUByte *) ((err == 1) ? "meSock Debug: SSL connection fully closed":"meSock Debug: SSL connection closing"),logData);
            }
            OPENSSLFunc(SSL_free)(sFp->ssl);
            sFp->ssl = NULL;
        }
#endif
        if(sFp->sck != meBadSocket)
        {
            meSocketClose(sFp->sck);
            sFp->sck = meBadSocket;
        }
        if(sFp->flags & meSOCK_CTRL_INUSE)
        {
            if(!force)
            {
                sFp->flags &= ~(meSOCK_INUSE|meSOCK_SHUTDOWN);
                sFp->length = -2;
                return 1;
            }
            if(sFp->flags & meSOCK_LOG_STATUS)
                logFunc(meSOCK_LOG_STATUS,(meUByte *) "Closing control channel",logData);
            if(!(sFp->flags & meSOCK_CTRL_NO_QUIT))
            {
                meUByte buff[meSOCK_BUFF_SIZE];
                meSockFtpCommand(sFp,buff,"QUIT");
            }
#if MEOPT_OPENSSL
            if(sFp->ctrlSsl != NULL)
            {
                if(sFp->flags & meSOCK_CTRL_SHUTDOWN)
                {
                    int err = OPENSSLFunc(SSL_shutdown)(sFp->ctrlSsl);
                    if(err < 0)
                    {
                        if(sFp->flags & meSOCK_LOG_WARNING)
                            logFunc(meSOCK_LOG_WARNING,(meUByte *) "meSock Warning: Control SSL shutdown failed",logData);
                    }
                    /* do not need to call SSL_shutdown again if err == 0 as we are closing the socket */
                    else if(sFp->flags & meSOCK_LOG_VERBOSE)
                        logFunc(meSOCK_LOG_VERBOSE,(meUByte *) ((err == 1) ? "meSock Debug: Control SSL connection fully closed":"meSock Debug: Control SSL connection closing"),logData);
                }
                OPENSSLFunc(SSL_free)(sFp->ctrlSsl);
                sFp->ctrlSsl = NULL;
            }
#endif
            if(sFp->ctrlSck != meBadSocket)
            {
                meSocketClose(sFp->ctrlSck);
                sFp->ctrlSck = meBadSocket;
            }
            if(sFp->home != NULL)
            {
                free(sFp->home);
                sFp->home = NULL;
            }
        }
        sFp->flags = 0;
    }
    return 0;
}


void
meSockEnd()
{
#if MEOPT_OPENSSL
    if(meSockCtx != NULL)
    {
        OPENSSLFunc(SSL_CTX_free)(meSockCtx);
        meSockCtx = NULL;
    }
#endif
#ifdef _WIN32
    if(WSAinit)
    {
        WSACleanup();
        WSAinit = 0;
    }
#endif
}
