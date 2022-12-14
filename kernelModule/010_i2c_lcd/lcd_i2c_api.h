#ifndef _LCD_I2C_API_H_
#define _LCD_I2C_API_H_

#define BUF_SIZE		64
#define ESC_SEQ_BUF_SIZE	4

struct hd44780_geometry {
	int cols;
	int rows;
	int start_addrs[];
};

/* just a struct describing for the LCD */ 
struct hd44780 {
	dev_t dev_number;
	struct cdev cdev;
	struct device *device;
	struct i2c_client *i2c_client;
	struct hd44780_geometry *geometry;

	/* Current cursor position on the display */
	struct {
		int row;
		int col;
	} pos;

	char buf[BUF_SIZE];
	struct {
		char buf[ESC_SEQ_BUF_SIZE];
		int length;
	} esc_seq_buf;
	bool is_in_esc_seq;

	bool backlight;
	bool cursor_blink;
	bool cursor_display;

	bool dirty;

	struct mutex lock;
	struct list_head list;
};

extern void hd44780_write(struct hd44780 *, const char *, size_t);
extern void hd44780_init_lcd(struct hd44780 *);
extern void hd44780_print(struct hd44780 *, const char *);
extern void hd44780_flush(struct hd44780 *);
extern void hd44780_set_geometry(struct hd44780 *, struct hd44780_geometry *);
extern void hd44780_set_backlight(struct hd44780 *, bool);
extern void hd44780_set_cursor_blink(struct hd44780 *, bool);
extern void hd44780_set_cursor_display(struct hd44780 *, bool);

extern struct hd44780_geometry *hd44780_geometries[];
#endif
