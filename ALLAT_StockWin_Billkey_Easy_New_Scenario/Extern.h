#include	"port.h"

////////////////////////////////////////////////////////////////////////
extern	int		debug;
extern	char	voc_path[];

extern	"C"		extern		VMSPARA vmspara;
extern	"C"		extern		LPMTP		*port, *lpmt;	// 회선 관리 정보	  
extern	"C"		extern		int		channel;

extern	MYLPMT	*mylpmt;
extern	MYLPMT	*xport[];

extern	int		hFixVoc[];		// Voice handle
extern	char	chDeviceLists[MAXCHAN][10];
extern	char	*chDeviceNames[];
extern	int		numbChDevice;
extern	BOOL	repeat;
extern	int		numThread;


extern	int		fLog;
extern	int		tstamp;
extern	int		timeout;

extern	char	recIP[];
extern	int		recPort;

extern	CWinIVRView*	pView;

extern	"C"		extern		void SessionCreate(int chan,char *pDins);