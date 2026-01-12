#include	"port.h"


extern	"C"		LPMTP		*port;		// 회선 관리 정보
MYLPMT	*mylpmt, *xport[MAXCHAN + 1];	// Local 회선 관리 정보

extern	"C"		extern	int		channel;
extern	"C"		extern	int		numchan;	// Number of Channels
extern	"C"		extern	LPMTP	*lpmt;		// 처리중 회선의 포인터
extern	"C"		extern	VMSPARA vmspara;

char	chDeviceLists[MAXCHAN][10];
char	*chDeviceNames[MAXCHAN];
int		numbChDevice;

int		hFixVoc[5];		// Voice handle

int		numThread = 0;	// Number of threads
BOOL	repeat = TRUE;	// Thread processing mode
int		debug = TRUE;	// Console debug mode

char	voc_path[80];	// Path of voice file

int		fLog;
int		tstamp;
int		timeout = 60;

char	recIP[20];
int		recPort;


