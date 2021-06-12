#pragma once

#include <folly/net/NetworkSocket.h>

namespace folly {
	namespace netops {
		int socketpair(int domain, int type, int protocol, NetworkSocket sv[2]);
	}
}