#include "NetOps.h"
#include <folly/portability/Windows.h>
#ifdef _WIN32
#include <MSWSock.h> // @manual
#endif

namespace folly {
    namespace netops {

#ifdef _WIN32

        //  adapted from like code in libevent, itself adapted from like code in tor
        //
        //  from: https://github.com/libevent/libevent/tree/release-2.1.12-stable
        //  license: 3-Clause BSD
        static int socketpair_win32(
            int family, int type, int protocol, intptr_t fd[2]) {
            intptr_t listener = -1;
            intptr_t connector = -1;
            intptr_t acceptor = -1;
            struct sockaddr_in listen_addr;
            struct sockaddr_in connect_addr;
            int size;
            int saved_errno = -1;
            int family_test;

            family_test = family != AF_INET && (family != AF_UNIX);
            if (protocol || family_test) {
                WSASetLastError(WSAEAFNOSUPPORT);
                return -1;
            }

            if (!fd) {
                WSASetLastError(WSAEINVAL);
                return -1;
            }

            listener = ::socket(AF_INET, type, 0);
            if (listener < 0) {
                return -1;
            }
            memset(&listen_addr, 0, sizeof(listen_addr));
            listen_addr.sin_family = AF_INET;
            listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            listen_addr.sin_port = 0; /* kernel chooses port.   */
            if (::bind(listener, (struct sockaddr*)&listen_addr, sizeof(listen_addr)) ==
                -1) {
                goto tidy_up_and_fail;
            }
            if (::listen(listener, 1) == -1) {
                goto tidy_up_and_fail;
            }

            connector = ::socket(AF_INET, type, 0);
            if (connector < 0) {
                goto tidy_up_and_fail;
            }

            memset(&connect_addr, 0, sizeof(connect_addr));

            /* We want to find out the port number to connect to.  */
            size = sizeof(connect_addr);
            if (::getsockname(listener, (struct sockaddr*)&connect_addr, &size) == -1) {
                goto tidy_up_and_fail;
            }
            if (size != sizeof(connect_addr)) {
                goto abort_tidy_up_and_fail;
            }
            if (::connect(
                connector, (struct sockaddr*)&connect_addr, sizeof(connect_addr)) ==
                -1) {
                goto tidy_up_and_fail;
            }

            size = sizeof(listen_addr);
            acceptor = ::accept(listener, (struct sockaddr*)&listen_addr, &size);
            if (acceptor < 0) {
                goto tidy_up_and_fail;
            }
            if (size != sizeof(listen_addr)) {
                goto abort_tidy_up_and_fail;
            }
            /* Now check we are talking to ourself by matching port and host on the
               two sockets.   */
            if (::getsockname(connector, (struct sockaddr*)&connect_addr, &size) == -1) {
                goto tidy_up_and_fail;
            }
            if (size != sizeof(connect_addr) ||
                listen_addr.sin_family != connect_addr.sin_family ||
                listen_addr.sin_addr.s_addr != connect_addr.sin_addr.s_addr ||
                listen_addr.sin_port != connect_addr.sin_port) {
                goto abort_tidy_up_and_fail;
            }
            ::closesocket(listener);
            fd[0] = connector;
            fd[1] = acceptor;

            return 0;

        abort_tidy_up_and_fail:
            saved_errno = WSAECONNABORTED;

        tidy_up_and_fail:
            if (saved_errno < 0) {
                saved_errno = WSAGetLastError();
            }
            if (listener != -1) {
                ::closesocket(listener);
            }
            if (connector != -1) {
                ::closesocket(connector);
            }
            if (acceptor != -1) {
                ::closesocket(acceptor);
            }

            WSASetLastError(saved_errno);
            return -1;
        }

#endif

        int socketpair(int domain, int type, int protocol, NetworkSocket sv[2]) {
#ifdef _WIN32
            if (domain != PF_UNIX || type != SOCK_STREAM || protocol != 0) {
                return -1;
            }
            intptr_t pair[2];
            auto r = socketpair_win32(AF_INET, type, protocol, pair);
            if (r == -1) {
                return r;
            }
            sv[0] = NetworkSocket(static_cast<SOCKET>(pair[0]));
            sv[1] = NetworkSocket(static_cast<SOCKET>(pair[1]));
            return r;
#elif defined(__XROS__)
            throw std::logic_error("Not implemented!");
#else
            int pair[2];
            auto r = ::socketpair(domain, type, protocol, pair);
            if (r == -1) {
                return r;
            }
            sv[0] = NetworkSocket(pair[0]);
            sv[1] = NetworkSocket(pair[1]);
            return r;
#endif
        }

    }
}