#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <chrono>
#include "picohttpparser.hpp"

#define REQ                                                                                                                        \
    "GET /wp-content/uploads/2010/03/hello-kitty-darth-vader-pink.jpg HTTP/1.1\r\n"                                                \
    "Host: www.kittyhell.com\r\n"                                                                                                  \
    "User-Agent: Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10.6; ja-JP-mac; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3 "             \
    "Pathtraq/0.9\r\n"                                                                                                             \
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"                                                  \
    "Accept-Language: ja,en-us;q=0.7,en;q=0.3\r\n"                                                                                 \
    "Accept-Encoding: gzip,deflate\r\n"                                                                                            \
    "Accept-Charset: Shift_JIS,utf-8;q=0.7,*;q=0.7\r\n"                                                                            \
    "Keep-Alive: 115\r\n"                                                                                                          \
    "Connection: keep-alive\r\n"                                                                                                   \
    "Cookie: wp_ozh_wsa_visits=2; wp_ozh_wsa_visit_lasttime=xxxxxxxxxx; "                                                          \
    "__utma=xxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.x; "                                                             \
    "__utmz=xxxxxxxxx.xxxxxxxxxx.x.x.utmccn=(referral)|utmcsr=reader.livedoor.com|utmcct=/reader/|utmcmd=referral\r\n"             \
    "\r\n"


int main(void)
{
    const char *method;
    size_t method_len;
    const char *path;
    size_t path_len;
    int minor_version;
    cinatra::http_header headers[64];
    size_t num_headers;
    int i, ret;

    float start, end;
    start = (float)clock()/CLOCKS_PER_SEC;

    for (i = 0; i < 5000000; i++) {
        num_headers = sizeof(headers) / sizeof(headers[0]);
        ret = cinatra::detail::phr_parse_request(REQ, sizeof(REQ) - 1, &method, &method_len, &path, &path_len, &minor_version, headers, &num_headers,
                                0);
        assert(ret == sizeof(REQ) - 1);
    }

    end = (float)clock()/CLOCKS_PER_SEC;
    
    printf("Elapsed %f seconds.\n", (end - start));

    /*
    基本功能测试
    num_headers = sizeof(headers) / sizeof(headers[0]);
    int pret = cinatra::detail::phr_parse_request(REQ, sizeof(REQ) - 1, &method, &method_len, &path, &path_len, &minor_version, headers, &num_headers,
                                0);
    printf("request is %d bytes long\n", pret);
    printf("method is %.*s\n", (int)method_len, method);
    printf("path is %.*s\n", (int)path_len, path);
    printf("HTTP version is 1.%d\n", minor_version);
    printf("num of headers is %ld\n", num_headers);
    printf("headers:\n");
    for (i = 0; i != num_headers; ++i)
    {
        std::cout << std::string(headers[i].name) << "header's value is: " << std::string(headers[i].value) << std::endl;
    }
    */

    return 0;
}
