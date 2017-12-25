/*

思路:直接传入一个驱动类金nfc_open()函数

nfc_open(viod *emdev_class)


typedef struct __emdev_class{
	void (*open)(void);
	int (*receive) (uint8_t *pbtRx, const size_t szRx, int timeout);
	int (*send) (const uint8_t *pbtTx, const size_t szTx, int timeout);
	void (*close)(void);
}emdev_class;

*/
