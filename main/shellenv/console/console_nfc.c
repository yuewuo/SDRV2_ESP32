#include "autoinclude.h"
#include "console_cmd.h"
#include "shellutil.h"
#include "nfc/nfc.h"
#include "libnfc/libnfc/buses/emdev.h"
#include <ctype.h>

static const char TAG[] = "console nfc";

static int nfc_console(int argc, char** argv);
void console_register_nfc() {
	const static esp_console_cmd_t cmd = {
		.command = "nfc",
		.help = "list init",
		.hint = NULL,
		.func = nfc_console,
	};
	ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

static nfc_emdev emdevs[UARTGrpCount];
static nfc_device* pn532s[UARTGrpCount];
static void scanprocess(uint8_t mask, nfc_device* pnd, int (*printf)(const char* format, ...));

#define ARGVCMP(x) (0 == strcmp(argv[i], x))
static int nfc_console(int argc, char** argv) {
	if (argc == 1) {
		Shell.Out.printf(nfc_version());
	}
	for (int i=1; i<argc; ++i) {
		if (ARGVCMP("list")) {
			if ((++i) == argc) {
				Shell.Out.printf("usage: nfc list [emdevs]");
			} else {
				if (ARGVCMP("emdevs")) {
					for (int j=0; j<2; ++j) {
						Shell.Out.printf("uart%d on emdev(%p)\n", j+1, emdevlistforuart[j]);
					}
				}
			}
		} else if (ARGVCMP("init")) {
			bool formatwrong = false;
			int rx = 0, tx = 0, baud = 0;
			if ((++i) == argc) formatwrong = true;
			#define getUartI(x) (UARTGrp.getIndex(x))
			while (i < argc && formatwrong == false) {
				if (ARGVCMP("-rx")) {
					if ((++i) == argc) { formatwrong = true; break; }
					rx = atoi(argv[i]);
				} else if (ARGVCMP("-tx")) {
					if ((++i) == argc) { formatwrong = true; break; }
					tx = atoi(argv[i]);
				} else if (ARGVCMP("-baud")) {
					if ((++i) == argc) { formatwrong = true; break; }
					baud = atoi(argv[i]);
				} else if (strlen(argv[i]) == 1 && isdigit((int)argv[i][0])) {
					int uart_num = atoi(argv[i]);
					UARTGrp_config_t* configp = UARTGrp.getConfig(uart_num);
					if (configp == NULL)  Shell.Out.printf("uart num %d invalid"
							" or has been occupied!\n", uart_num);
					configp->tx_io_num = tx != 0 ? tx : configp->tx_io_num;
					configp->rx_io_num = rx != 0 ? rx : configp->rx_io_num;
					if (baud != 0) configp->uart_config.baud_rate = baud;
					getUartGrpforNFCemdev(&(emdevs[getUartI(uart_num)]), uart_num);
					emdev_init(&(emdevs[UARTGrp.getIndex(uart_num)]));
					UARTGrp.printInitInfo(Shell.Out.printf, uart_num);
					nfc_device *pnd = pn532_open(&(emdevs[getUartI(uart_num)]));
					if (pnd == NULL) {
						Shell.Out.printf("Unable to open NFC device\n");
						return -1;
					}
					if (nfc_initiator_init(pnd) < 0) {
						Shell.Out.printf("nfc initiator init failed\n");
						return -1;
					}
					printf("NFC device: %s opened\n", nfc_device_get_name(pnd));
					pn532s[getUartI(uart_num)] = pnd;
					break;
				} else {
					formatwrong = true; break;
				} ++i;
			}
			if (formatwrong) Shell.Out.printf("usage: nfc init [(-rx RXFOOT), "
					"(-tx TXFOOT), (-baud BAUDWIDTH), (1/2)]\n");
		} else if (ARGVCMP("scan")) {
			bool formatwrong = false;
			char mask = 0xff;
			if ((++i) == argc) formatwrong = true;
			#define getUartI(x) (UARTGrp.getIndex(x))
			while (i < argc && formatwrong == false) {
				if (ARGVCMP("-m")) {
					if ((++i) == argc || strlen(argv[i]) != 2) { formatwrong = true; break; }
					mask = htoi2(argv[i]);
				} else if (strlen(argv[i]) == 1 && isdigit((int)argv[i][0])) {
					int uart_num = atoi(argv[i]);
					scanprocess(mask, pn532s[getUartI(uart_num)], Shell.Out.printf);
					break;
				} else {
					formatwrong = true; break;
				} ++i;
			}
			if (formatwrong) {
				Shell.Out.printf("usage: nfc scan [(-m MASK), "
					"(1/2)]\n");
				Shell.Out.printf("\t   1: ISO14443A\n");
	  			Shell.Out.printf("\t   2: Felica (212 kbps)\n");
	  			Shell.Out.printf("\t   4: Felica (424 kbps)\n");
	  			Shell.Out.printf("\t   8: ISO14443B\n");
	  			Shell.Out.printf("\t  16: ISO14443B'\n");
	  			Shell.Out.printf("\t  32: ISO14443B-2 ST SRx\n");
	  			Shell.Out.printf("\t  64: ISO14443B-2 ASK CTx\n");
	  			Shell.Out.printf("\t 128: Jewel\n");
	  			Shell.Out.printf("\tSo 255 (default) polls for all types.\n");
	  			Shell.Out.printf("\tNote that if 16, 32 or 64 then 8 is selected too.\n");
			}
		}
	}
	return 0;
}

#define print_nfc_target(pnt, verbose) do {\
	char *s;\
  	str_nfc_target(&s, pnt, verbose);\
  	printf("%s", s);\
  	nfc_free(s);\
} while(0);
static void scanprocess(uint8_t mask, nfc_device* pnd, int (*printf)(const char* format, ...)) {
	nfc_modulation nm;
	int res = 0;
	#define verbose false
	#define MAX_TARGET_COUNT 16
	nfc_target ant[MAX_TARGET_COUNT];

	    if (mask & 0x1) {
	      nm.nmt = NMT_ISO14443A;
	      nm.nbr = NBR_106;
	      // List ISO14443A targets
	      if ((res = nfc_initiator_list_passive_targets(pnd, nm, ant, MAX_TARGET_COUNT)) >= 0) {
	        int n;
	        if (verbose || (res > 0)) {
	          printf("%d ISO14443A passive target(s) found%s\n", res, (res == 0) ? ".\n" : ":");
	        }
	        for (n = 0; n < res; n++) {
	          print_nfc_target(&ant[n], verbose);
	          printf("\n");
	        }
	      }
	    }

	    if (mask & 0x02) {
	      nm.nmt = NMT_FELICA;
	      nm.nbr = NBR_212;
	      // List Felica tags
	      if ((res = nfc_initiator_list_passive_targets(pnd, nm, ant, MAX_TARGET_COUNT)) >= 0) {
	        int n;
	        if (verbose || (res > 0)) {
	          printf("%d Felica (212 kbps) passive target(s) found%s\n", res, (res == 0) ? ".\n" : ":");
	        }
	        for (n = 0; n < res; n++) {
	          print_nfc_target(&ant[n], verbose);
	          printf("\n");
	        }
	      }
	    }

	    if (mask & 0x04) {
	      nm.nmt = NMT_FELICA;
	      nm.nbr = NBR_424;
	      if ((res = nfc_initiator_list_passive_targets(pnd, nm, ant, MAX_TARGET_COUNT)) >= 0) {
	        int n;
	        if (verbose || (res > 0)) {
	          printf("%d Felica (424 kbps) passive target(s) found%s\n", res, (res == 0) ? ".\n" : ":");
	        }
	        for (n = 0; n < res; n++) {
	          print_nfc_target(&ant[n], verbose);
	          printf("\n");
	        }
	      }
	    }

	    if (mask & 0x08) {
	      nm.nmt = NMT_ISO14443B;
	      nm.nbr = NBR_106;
	      // List ISO14443B targets
	      if ((res = nfc_initiator_list_passive_targets(pnd, nm, ant, MAX_TARGET_COUNT)) >= 0) {
	        int n;
	        if (verbose || (res > 0)) {
	          printf("%d ISO14443B passive target(s) found%s\n", res, (res == 0) ? ".\n" : ":");
	        }
	        for (n = 0; n < res; n++) {
	          print_nfc_target(&ant[n], verbose);
	          printf("\n");
	        }
	      }
	    }

	    if (mask & 0x10) {
	      nm.nmt = NMT_ISO14443BI;
	      nm.nbr = NBR_106;
	      // List ISO14443B' targets
	      if ((res = nfc_initiator_list_passive_targets(pnd, nm, ant, MAX_TARGET_COUNT)) >= 0) {
	        int n;
	        if (verbose || (res > 0)) {
	          printf("%d ISO14443B' passive target(s) found%s\n", res, (res == 0) ? ".\n" : ":");
	        }
	        for (n = 0; n < res; n++) {
	          print_nfc_target(&ant[n], verbose);
	          printf("\n");
	        }
	      }
	    }

	    if (mask & 0x20) {
	      nm.nmt = NMT_ISO14443B2SR;
	      nm.nbr = NBR_106;
	      // List ISO14443B-2 ST SRx family targets
	      if ((res = nfc_initiator_list_passive_targets(pnd, nm, ant, MAX_TARGET_COUNT)) >= 0) {
	        int n;
	        if (verbose || (res > 0)) {
	          printf("%d ISO14443B-2 ST SRx passive target(s) found%s\n", res, (res == 0) ? ".\n" : ":");
	        }
	        for (n = 0; n < res; n++) {
	          print_nfc_target(&ant[n], verbose);
	          printf("\n");
	        }
	      }
	    }

	    if (mask & 0x40) {
	      nm.nmt = NMT_ISO14443B2CT;
	      nm.nbr = NBR_106;
	      // List ISO14443B-2 ASK CTx family targets
	      if ((res = nfc_initiator_list_passive_targets(pnd, nm, ant, MAX_TARGET_COUNT)) >= 0) {
	        int n;
	        if (verbose || (res > 0)) {
	          printf("%d ISO14443B-2 ASK CTx passive target(s) found%s\n", res, (res == 0) ? ".\n" : ":");
	        }
	        for (n = 0; n < res; n++) {
	          print_nfc_target(&ant[n], verbose);
	          printf("\n");
	        }
	      }
	    }

	    if (mask & 0x80) {
	      nm.nmt = NMT_JEWEL;
	      nm.nbr = NBR_106;
	      // List Jewel targets
	      if ((res = nfc_initiator_list_passive_targets(pnd, nm, ant, MAX_TARGET_COUNT)) >= 0) {
	        int n;
	        if (verbose || (res > 0)) {
	          printf("%d Jewel passive target(s) found%s\n", res, (res == 0) ? ".\n" : ":");
	        }
	        for (n = 0; n < res; n++) {
	          print_nfc_target(&ant[n], verbose);
	          printf("\n");
	        }
	      }
	    }
}
