#include "router.h"

int main() {
    Router router(8080, "127.0.0.1", 9090); // Listen on 8080, forward to 127.0.0.1:9090
    router.startRouting();
    return 0;
}
