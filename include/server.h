


typedef struct clientList
{
	int id;
	char hostname[256];
	char IPaddress[100];
	int port;
	int logStatus;
	int mSent;
	int mReceived;
	int isBlocked;
	char blockedby[100];
	struct clientList *next;
} clientList;

typedef struct peerSideList
{
	char hostname[256];
	char IPaddress[100];
	int port;
	int isBlocked;
	struct peerSideList *next;
} peerSideList;
