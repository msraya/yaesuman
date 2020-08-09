#if !defined(WIRESX_H)
#define	WIRESX_H

#include <vector>
#include <string>
#include <stdint.h>

#define T_READ true
#define T_WRITE false

typedef struct Main_Config {
	char callsign[17];
	char Gps[21];
} MainConfig;

struct Txt_Node {
	uint16_t mark;       
	uint16_t index;   		//4 bytes
	uint8_t repeater[5];   	//5 bytes ASCII
	uint8_t To[16]; 		//16 bytes ASCII
	uint8_t signature[5]; 	//5 bytes
	uint8_t From[16];  		//16 bytes ASCII
	uint8_t Date_Crea[6];   //Compressed Date of creation
	uint8_t Date_Send[6];   //Compressed Date send	
	uint8_t Date_Rcv[6];	//Compressed Date received
	uint8_t subject[16];	//unused fill with 0x20
	uint32_t pos;			//position in file QSOMSG	
	uint8_t info[16];		//unused fill with 0x20
	uint8_t Gps[20];		//Gps information
	uint8_t fill[8];		//unused fill 0x00
	uint8_t text[80];		//Message
	struct Txt_Node *next;			//linked list
};  //ALL 128bytes + linked pointer

typedef struct Txt_Node TxtNode;

struct Pct_Node {
	uint16_t mark;           // 0x69
	uint16_t index;   		// increment in two units
	uint8_t repeater[5];   	//5 bytes ASCII
	uint8_t To[16]; 		//16 bytes ASCII
	uint8_t signature[5]; 	//5 bytes
	uint8_t From[16];  		//16 bytes ASCII
	uint8_t Date_Crea[6];   //Compressed Date of creation
	uint8_t Date_Send[6];   //Compressed Date send	
	uint8_t Date_Rcv[6];	//Compressed Date received
	uint8_t subject[16];	//fill with date 2018/12/13 17:22
	uint32_t size;			//size of JPEG file
	uint8_t filename[16];	//picture filename
	uint8_t Gps[20];		//Gps information
	uint8_t fill[8];		//unused fill 0x00
	struct Pct_Node *next;			//linked list
};  //ALL 128bytes + linked pointer

typedef struct Pct_Node PctNode;

class CData {
public:
	CData(char *callsign, char *Gps);
	~CData();
	
	bool LoadFile(const char *file_name);
	bool SaveFile(const char *file_name);
	
	void config(char *callsign, char *Gps);
	
	void reset(void);
	void next(void);	
	bool item_left(void);

	void preset(void);
	void pnext(void);	
	bool pitem_left(void);
		
	void date(char *date, bool read);
	void from(char *from, bool read);
	void to(char *to, bool read);
	void pdate(char *date, bool read);
	void pfrom(char *from, bool read);
	void pto(char *to, bool read);	
	void repeater(char *repeat, bool read);
	void prepeater(char *repeat, bool read);	
	void position(char *position, bool read);
	void pposition(char *position, bool read);	
	void subject(char *subject, bool read);
	void psubject(char *subject, bool read);	
	void text_data(char *text, bool read);		
	void filename(char *filenam, bool read);
	void get_filename(char *filen);
		
	void go_to(uint16_t pos);
	void remove_node(uint16_t number);
	void add_node(uint8_t *data);
	uint16_t num_nodes(void);
	
	void go_pto(uint16_t pos);
	void remove_pnode(uint16_t number);
	void add_pnode(uint8_t *data);
	uint16_t num_pnodes(void);
	bool changed(void);		

	char *getFileName(const char *path);
	void getBaseName(char *buffer, const char *path);
	
	void clear_nodes(void);
	void clear_pnodes(void);
	
	void remove_dup(void);
	void remove_pdup(void);	
		
private:
	TxtNode *		m_head;
	TxtNode *		m_tail;
	TxtNode *		m_actual;
	uint16_t		m_nodes;
	PctNode *		m_phead;
	PctNode *		m_ptail;
	PctNode *		m_pactual;
	uint16_t		m_pnodes;
	uint16_t		m_pictures;			
	MainConfig 		m_config;
	bool			m_changed;	
	void dump(char *buffer, uint8_t *data,uint16_t size);
};

#endif
