/*
 * winsock2_compat.h
 * ------------------
 *
 * Local replacement for the system winsock2.h header.
 *
 * The very old winsock2.h that ships with Dev-C++ / old MinGW
 * contains an unbalanced #endif around line 46, which causes
 * GCC to abort compilation the moment the header is processed.
 *
 * This shim declares only the Winsock types and functions that
 * the HTTP layer of this application actually uses.  It is
 * included instead of <winsock2.h> so that the broken system
 * header is never pulled in.
 *
 * The declarations mirror the native Win32 WinSock API and are
 * ABI-compatible with ws2_32.dll on Windows 2000.
 *
 * Link with:  -lws2_32
 */

#ifndef CLASSICAL_WS2_COMPAT_H
#define CLASSICAL_WS2_COMPAT_H

/*
 * SOCKET
 *
 * On Win32 LLP64, SOCKET is a 32-bit unsigned integer
 * (same size as int).  ws2_32.dll functions accept and
 * return this type; 0xFFFFFFFF is the error sentinel.
 */
typedef unsigned int SOCKET;

#define INVALID_SOCKET ((SOCKET)(~0))

/*
 * Protocol family and socket type constants
 * (match ws2_32.dll values on Windows 2000).
 */
#define AF_INET     2
#define SOCK_STREAM 1

/*
 * Byte-order conversion macros.
 *
 * On Win32 x86, htons/ntohs are byte-swap operations.
 * They are normally defined as macros in winsock2.h.
 * Here we define them directly to avoid a link dependency
 * on ws2_32.dll exports that do not exist as named symbols.
 */
#define htons(x)  ((unsigned short)( \
    (((unsigned short)(x) & 0xff00) >> 8) | \
    (((unsigned short)(x) & 0x00ff) << 8) \
))
#define ntohs(x)  htons(x)

/*
 * Winsock address structures (ABI-matches ws2_32.dll).
 */

/* SOCKADDR */
struct sockaddr {
    unsigned short sa_family;
    unsigned char  sa_data[14];
};
typedef struct sockaddr SOCKADDR;

/* SOCKADDR_IN (IPv4) */
struct sockaddr_in {
    unsigned short sin_family;
    unsigned short sin_port;
    unsigned int   sin_addr;   /* 32-bit IPv4, network byte order */
    char           sin_zero[8];
};
typedef struct sockaddr_in SOCKADDR_IN;

/*
 * WSADATA
 *
 * Minimal layout; WSAStartup only needs the version fields.
 * The struct is passed by pointer, so the OS fills it in.
 */
typedef struct {
    unsigned short  wVersion;
    unsigned short  wHighVersion;
    char            szWSACat[256];   /* padding area */
} WSADATA;

/*
 * in_addr
 *
 * 32-bit IPv4 address, network byte order.
 */
typedef unsigned int in_addr;

/*
 * hostent
 *
 * Minimal layout matching ws2_32.dll.  gethostbyname
 * returns a pointer to a static struct populated by
 * the OS; we only read h_length and h_addr_list.
 */
struct hostent {
    char *h_name;
    char *h_aliases;
    int   h_addrtype;
    int   h_length;
    char **h_addr_list;
};

/*
 * Function prototypes (extern "C" for C++ linkage).
 */

#ifdef __cplusplus
extern "C" {
#endif

/* WSAStartup / WSACleanup */
int  WSAStartup(int wVersionRequested, WSADATA *lpWSAData);
int  WSACleanup(void);

/* Host name resolution (legacy, available on Windows 2000) */
struct hostent *gethostbyname(const char *name);

/* Socket creation and I/O */
SOCKET socket(int af, int type, int protocol);
int    closesocket(SOCKET s);
int    connect(SOCKET s, const struct sockaddr *name, int namelen);
int    send(SOCKET s, const void *buf, int len, int flags);
int    recv(SOCKET s, void *buf, int len, int flags);

/* Error code */
int  WSAGetLastError(void);

#ifdef __cplusplus
}
#endif

/*
 * Winsock error codes used by this project.
 */
#define WSAEWOULDBLOCK  10035

#endif /* CLASSICAL_WS2_COMPAT_H */
