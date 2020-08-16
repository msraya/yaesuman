#include <windows.h>
#include "wiresx.h"
#include <math.h>

#include <algorithm>
#include <functional>
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cctype>

char m_signature[7U];

static void strcpyfill(uint8_t *buf,char *cad, uint16_t size){
	uint16_t i,j,n;
	
	if (strlen(cad)==0) {
		memset(buf,0x20,size);
	} else {
		n = strlen(cad);		
		memcpy(buf,cad,n);
		i = size-n;
		for (j=0;j<i;j++) buf[n+j]=0x20;
	}
}

CData::CData(char *callsign, char *Gps, char *Quality)
{
    m_head = NULL;
    m_tail = NULL;
    m_actual = NULL;
    m_nodes = 0;
	m_phead = NULL;
	m_ptail = NULL;
	m_pactual = NULL;
	m_pnodes = 0; 
	m_changed = false;   

    strcpyfill((uint8_t *)m_config.callsign,callsign,16U);
    if (strlen(Gps) == 20U) strcpy(m_config.Gps,Gps);
    else memset(m_config.Gps,0x00,20U);
    strcpy(m_config.Quality,Quality);
    //MessageBox(0, m_config.Gps, "Oh Oh...", MB_ICONEXCLAMATION | MB_OK);
}

void CData::config(char *callsign, char *Gps, char *Quality){
    strcpyfill((uint8_t *)m_config.callsign,callsign,16U);
    if (strlen(Gps) == 20U) strcpy(m_config.Gps,Gps);
    else memset(m_config.Gps,0x00,20U);
    strcpy(m_config.Quality,Quality);
}

CData::~CData()
{
	clear_nodes();
	clear_pnodes();
}

uint8_t file_buf[208U];
char mng_name[100U];
char fat_name[100U];
char msg_name[100U];
char pfat_name[100U];
char pdir_name[100U];
char tmp_cad[128];
char m_basename[512];
char cadena[512];
	
char *CData::getFileName(const char *path) {
	char *ssc;
	char *buf;
	int l = 0;	
	
	strcpy(tmp_cad,path);
	buf=tmp_cad;	
	ssc = strstr(buf, "\\");
	if (ssc==NULL) return buf;
	do{
	    l = strlen(ssc) + 1;
	    buf = &buf[strlen(buf)-l+2];
	    ssc = strstr(buf, "\\");
	} while(ssc);
	return buf;
}

void CData::getBaseName(char *buffer, const char *path) {
	char *ssc;
	char *buf;
	uint16_t l=0,len;
	
	strcpy(tmp_cad,path);
	buf=tmp_cad;	
	ssc = strstr(buf, "\\");
	if (ssc==NULL) {
		strcpy(buffer,"");
		return;
	}	
	do{
	    l = strlen(ssc) + 1;
	    buf = &buf[strlen(buf)-l+2];
	    ssc = strstr(buf, "\\");
	} while(ssc);
	len=strlen(path)-strlen(buf);
	strncpy(buffer,path,len);
	buffer[len]=0;
}
	
__int64 FileSize(LPCSTR name) {
//	std::string strFile;	
//	std::wstring strwFile;
//	
//	strFile = std::string(name);
//	strwFile = std::wstring( strFile.begin(), strFile.end() );	
	
    HANDLE hFile = CreateFile(name, GENERIC_READ, 
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile==INVALID_HANDLE_VALUE)
        return -1; // error condition, could call GetLastError to find out more

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size))
    {
        CloseHandle(hFile);
        return -1; // error condition, could call GetLastError to find out more
    }

    CloseHandle(hFile);
    return size.QuadPart;
}


char instructions[] = 	"For this program to work you must follow the instruction:\n"
						"   (1) Power off Transceiver. Remove SD Card. Insert\n"
						"       SDCard into PC.\n"
						"   (2) Make backup of SDCARD. You could screw up your\n"
						"       files easily.\n"
						"   (3) Locate QSOLOG folder. Only QSOLOG and PHOTO \n"
						"       folders are used.\n"
						"   (4) Select any .DAT file. All .DAT files are used except\n"
						"       WAV files in FT-3D transceiver.\n"
						"   (5) Modify photo or messages and hit Save and Exit button.\n"
						"       Only when you hit save information will be saved.\n"
						"   (6) Pictures are see as get by the camera. Enjoy.\n"
						"       73s from EA7EE.";

bool CData::LoadFile(const char *file_name)
{
FILE *file_dir;
FILE *file_fat;
FILE *file_msg;
uint16_t count,c,tmp_nodes,tmp_pnodes;
uint32_t pos;
char buf[4];
bool get_key;

	get_key=false;
	// Get Basename for photo
	getBaseName(m_basename,file_name);
	if (strlen(m_basename)>7) {
		strncpy(fat_name,m_basename,strlen(m_basename)-7);
		strcpy(m_basename,fat_name);
	}
	strcat(m_basename,"PHOTO\\");
	//MessageBox(0, m_basename, "Error", MB_ICONEXCLAMATION | MB_OK);
	// Open main index file
	getBaseName(msg_name,file_name);
	strcat(msg_name,"QSOMNG.DAT");
	//MessageBox(0, msg_name, "Error", MB_ICONEXCLAMATION | MB_OK);
	if ((file_msg=fopen(msg_name,"rb")) == NULL) {
			MessageBox(0, instructions, "Error", MB_ICONEXCLAMATION | MB_OK);
			return true;
	}
	count=fread(file_buf,1U,32U,file_msg);
	if (count!=32U) {
		MessageBox(0, instructions, "Error", MB_ICONEXCLAMATION | MB_OK);
		fclose(file_msg);
	 	return true;
	}
	fclose(file_msg);
	// Get number of messages and pictures
	if ((file_buf[0]==0xFF) && (file_buf[1]==0xFF))	tmp_nodes = 0;
	else tmp_nodes = file_buf[0]*256+file_buf[1];

	if ((file_buf[16]==0xFF) && (file_buf[17]==0xFF)) tmp_pnodes = 0;
	else tmp_pnodes = file_buf[16]*256+file_buf[17];
	m_pictures = file_buf[18]*256+file_buf[19];

	// return if not message is inside
	if (tmp_nodes>0) {
		// Get messages into linked list
		getBaseName(fat_name,file_name);
		strcat(fat_name,"QSOMSGFAT.DAT");
		if ((file_fat=fopen(fat_name,"rb")) == NULL) {
			MessageBox(0, "File QSOMSGFAT.DAT not found", "Error", MB_ICONEXCLAMATION | MB_OK);
			return true;
		}
		getBaseName(fat_name,file_name);
		strcat(fat_name,"QSOMSGDIR.DAT");	
		if ((file_dir=fopen(fat_name,"rb")) == NULL) {
			MessageBox(0, "File QSOMSGDIR.DAT not found", "Error", MB_ICONEXCLAMATION | MB_OK);
			fclose(file_fat);
			return true;
		}
		getBaseName(msg_name,file_name);
		strcat(msg_name,"QSOMSG.DAT");
		m_pos_msg = FileSize(msg_name);
		sprintf(cadena,"Pos : %04X",m_pos_msg);
		//MessageBox(0, msg_name, "Error", MB_ICONEXCLAMATION | MB_OK);
		if ((file_msg=fopen(msg_name,"rb")) == NULL) {
			MessageBox(0, "File QSOMSG.DAT not found", "Error", MB_ICONEXCLAMATION | MB_OK);
			fclose(file_dir);
			fclose(file_fat);			
			return true;
		}

		count=fread(file_buf,1U,128U,file_dir);
		if (count!=128U) {
			MessageBox(0, "Error with QSOMSGDIR.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
			fclose(file_msg);
			fclose(file_fat);		
		 	fclose(file_dir);
		 	return true;
		}
		
		do {
			c=fread(buf,1U,4U,file_fat);
			if (c!=4U) {
				MessageBox(0, "Error with QSOMSGFAT.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
				fclose(file_msg);
				fclose(file_fat);		
				fclose(file_dir);
				return true;				
			}
			if (buf[0]!=0x40) {
				count=fread(file_buf,1U,128U,file_dir);
				continue;
			}
			pos=((uint32_t)file_buf[0x52])*256+file_buf[0x53];
			fseek(file_msg,pos,SEEK_SET);
			c=fread(file_buf+128U,1U,80U,file_msg);
			if (c!=80U) {
				MessageBox(0, "Error with QSOMSG.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);  //"Error with QSOMSG.DAT"
				fclose(file_msg);
				fclose(file_fat);		
				fclose(file_dir);
				return true;				
			}
			add_node(file_buf);					
			count=fread(file_buf,1U,128U,file_dir);	
		} while (count==128U);
		
		fclose(file_msg);
		fclose(file_fat);	 
		fclose(file_dir);
	} else {
		m_pos_msg = 0;
	}
	
	if (tmp_pnodes>0) {
		getBaseName(fat_name,file_name);
		strcat(fat_name,"QSOPCTFAT.DAT");		
		if ((file_fat=fopen(fat_name,"rb")) == NULL) {
				MessageBox(0, "File QSOPCTFAT.DAT not found", "Error", MB_ICONEXCLAMATION | MB_OK);
				return true;
		}	
		getBaseName(fat_name,file_name);
		strcat(fat_name,"QSOPCTDIR.DAT");	
		if ((file_dir=fopen(fat_name,"rb")) == NULL) {
				MessageBox(0, "File QSOPCTDIR.DAT not found", "Error", MB_ICONEXCLAMATION | MB_OK);
				fclose(file_fat);
				return true;
		}
		count=fread(file_buf,1U,128U,file_dir);
		if (count!=128U) {
			MessageBox(0, "Error with QSOPCTDIR.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
			fclose(file_fat);		
		 	fclose(file_dir);
		 	return true;
		}

		// Copy signature
		memcpy(m_signature,file_buf+0x54,6U);			
		do {
			c=fread(buf,1U,4U,file_fat);
			if (c!=4U) {
				MessageBox(0, "Error with QSOPCTFAT.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
				fclose(file_fat);		
				fclose(file_dir);
				return true;				
			}
			if (buf[0]!=0x40) {
				count=fread(file_buf,1U,128U,file_dir);
				continue;
			}
			add_pnode(file_buf);					
			count=fread(file_buf,1U,128U,file_dir);	
		} while (count==128U);
	 	fclose(file_fat);
	 	fclose(file_dir);
	} else memcpy(m_signature,"HE5Gbv",6U);
	
	
 	m_changed = false;

 	return false;
}

uint8_t buf_man[32];

bool CData::SaveFile(const char *file_name)
{
FILE *file_dir;
FILE *file_fat;
FILE *file_msg;
FILE *file_man;
uint16_t c,pos1,pos2;
uint32_t pos;
char buf[4];
unsigned char *ptr;
TxtNode *tmp_node;
PctNode *tmp_pnode;

	getBaseName(mng_name,file_name);
	strcat(mng_name,"QSOMNG.DAT");
	if ((file_man=fopen(mng_name,"wb")) == NULL) {
			MessageBox(0, "Error with QSOMNG.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
			return true;
	}
	memset(buf_man,0xFF,32U);
	if (m_nodes==0) {buf_man[0]=0xFF;buf_man[1]=0xFF;}
	else {
		buf_man[0]=m_nodes/256;
		buf_man[1]=m_nodes%256;
	}
	if (m_pnodes==0) {buf_man[16]=0xFF;buf_man[17]=0xFF;}
	else {
		buf_man[16]=m_pnodes/256;
		buf_man[17]=m_pnodes%256;
	}
	buf_man[18]=m_pictures/256;
	buf_man[19]=m_pictures%256;		
	c=fwrite(buf_man,1U,32U,file_man);
	if (c!=32U) {
		MessageBox(0, "Error with QSOMNG.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
		fclose(file_msg);
		fclose(file_fat);		
		fclose(file_dir);
		return true;		
	}	
	fclose(file_man);

	getBaseName(fat_name,file_name);
	strcat(fat_name,"QSOMSGFAT.DAT");
	if ((file_fat=fopen(fat_name,"wb")) == NULL) {
			MessageBox(0, "Error with QSOMSGFAT.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
			return true;
	}

	getBaseName(msg_name,file_name);
	strcat(msg_name,"QSOMSG.DAT");
	if ((file_msg=fopen(msg_name,"wb")) == NULL) {
			MessageBox(0, "Error with QSOMSG.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
			fclose(file_fat);
			return true;
	}
	getBaseName(fat_name,file_name);
	strcat(fat_name,"QSOMSGDIR.DAT");	
	if ((file_dir=fopen(fat_name,"wb")) == NULL) {
			MessageBox(0, "Error with QSOMSGDIR.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
			fclose(file_msg);
			fclose(file_fat);
			return true;
	}
	
	tmp_node = m_head;
	pos1=0;
//	pos2=0;
	while (tmp_node!=NULL) {
		// We start with FAT file		
		buf[0]=0x40;
		buf[1]=0x00;
		buf[2]=pos1/256;			
		buf[3]=pos1%256;
		pos1+=0x80;	
		c=fwrite(buf,1U,4U,file_fat);
		if (c!=4U) {
			MessageBox(0, "Error with QSOMSGFAT.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
			fclose(file_msg);
			fclose(file_fat);		
			fclose(file_dir);
			return true;		
		}
		ptr = (unsigned char *)tmp_node;
//		//ptr[3U] = pos1/0x80;
//		ptr[0x52] = pos2/256;
//		ptr[0x53] = pos2%256;
//		pos2+=0x50;	
		c=fwrite(ptr,1U,128U,file_dir);
		if (c!=128U) {
			MessageBox(0, "Error with QSOMSGDIR.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
			fclose(file_msg);
			fclose(file_fat);		
			fclose(file_dir);
			return true;		
		}
		// pos of message in message file
		pos=((uint32_t)ptr[0x52])*256+ptr[0x53];
		fseek(file_msg,pos,SEEK_SET);		
		ptr+=128U;		
		c=fwrite((void *)ptr,1U,80U,file_msg);
		if (c!=80U) {
			MessageBox(0, "Error with QSOMSG.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
			fclose(file_msg);
			fclose(file_fat);		
			fclose(file_dir);
			return true;		
		}			
		tmp_node=tmp_node->next;	
	}
	
	fclose(file_msg);
	fclose(file_fat);		
	fclose(file_dir);

	getBaseName(fat_name,file_name);
	strcat(fat_name,"QSOPCTFAT.DAT");
	if ((file_fat=fopen(fat_name,"wb")) == NULL) {
			MessageBox(0, "Error with QSOPCTFAT.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
			return true;
	}

	getBaseName(fat_name,file_name);
	strcat(fat_name,"QSOPCTDIR.DAT");	
	if ((file_dir=fopen(fat_name,"wb")) == NULL) {
			MessageBox(0, "Error with QSOPCTDIR.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
			fclose(file_msg);
			fclose(file_fat);
			return true;
	}
	
	tmp_pnode = m_phead;
	pos1=0;
	pos2=0;
	while (tmp_pnode!=NULL) {
		// We start with FAT file		
		buf[0]=0x40;
		buf[1]=0x00;
		buf[2]=pos1/256;			
		buf[3]=pos1%256;
		pos1+=0x80;	
		c=fwrite(buf,1U,4U,file_fat);
		if (c!=4U) {
			MessageBox(0, "Error with QSOPCTFAT.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
			fclose(file_fat);		
			fclose(file_dir);
			return true;		
		}
		ptr = (unsigned char *)tmp_pnode;
//		ptr[2] = pos2/256;
//		ptr[3] = pos2%256;
//		pos2++;	
		c=fwrite(ptr,1U,128U,file_dir);
		if (c!=128U) {
			MessageBox(0, "Error with QSOPCTDIR.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
			fclose(file_msg);
			fclose(file_fat);		
			fclose(file_dir);
			return true;		
		}
		tmp_pnode=tmp_pnode->next;	
	}
	
	fclose(file_fat);		
	fclose(file_dir);
	
return false;
}


void CData::reset(void){
	m_actual = m_head;
}

bool CData::item_left( ) {
	return (m_actual != NULL);
}

void CData::next(void) {
	m_actual = m_actual->next;
}

void CData::preset(void){
	m_pactual = m_phead;
}

bool CData::pitem_left( ) {
	return (m_pactual != NULL);
}

void CData::pnext(void) {
	m_pactual = m_pactual->next;
}

void CData::go_to(uint16_t pos) {
	uint16_t i=pos;
	
	m_actual = m_head;
	while ((i>0) && ((m_actual->next)!=NULL)) {
		m_actual = m_actual->next;
		i--;	
	}
}

void CData::go_pto(uint16_t pos){
	uint16_t i=pos;
	
	m_pactual = m_phead;
	while ((i>0) && ((m_pactual->next)!=NULL)) {
		m_pactual = m_pactual->next;
		i--;	
	}	
}

uint16_t num_pnodes(void);			

#define bcd2bin(val) ((val)&15) + ((val)>>4)*10
#define bin2bcd(val) (((val)/10)<<4) + (val)%10

void CData::date(char *date, bool read) {
	char ptr[6];
	int year,month,day,hour,min,sec;
	
	if (read) {
		if (m_actual!=NULL) {
			memcpy(ptr,m_actual->Date_Send,6U);
			sprintf(date,"%02d/%02d/%04d %02d:%02d:%02d",bcd2bin(ptr[2]),bcd2bin(ptr[1]),
			2000U+bcd2bin(ptr[0]),bcd2bin(ptr[3]),bcd2bin(ptr[4]),bcd2bin(ptr[5]));
		} else strcpy(date,"----/--/-- --:--:--");
	} else {
		m_changed = true;
		if (m_actual!=NULL) {
			sscanf(date,"%02d/%02d/%04d %02d:%02d:%02d",&day,&month,&year,&hour,&min,&sec);
			ptr[0]=bin2bcd(year-2000U);
			ptr[1]=bin2bcd(month);
			ptr[2]=bin2bcd(day);
			ptr[3]=bin2bcd(hour);
			ptr[4]=bin2bcd(min);
			ptr[5]=0;
			memcpy(m_actual->Date_Crea,ptr,6U);
			memcpy(m_actual->Date_Send,ptr,6U);		
			memcpy(m_actual->Date_Rcv,ptr,6U);				
		}
	}
}

void CData::pdate(char *date, bool read) {
	char ptr[6];
	int year,month,day,hour,min,sec;
	
	if (read) {
		if (m_pactual!=NULL) {
			memcpy(ptr,m_pactual->Date_Send,6U);
			sprintf(date,"%02d/%02d/%04d %02d:%02d:%02d",bcd2bin(ptr[2]),bcd2bin(ptr[1]),
			2000U+bcd2bin(ptr[0]),bcd2bin(ptr[3]),bcd2bin(ptr[4]),bcd2bin(ptr[5]));
		} else strcpy(date,"----/--/-- --:--:--");
	} else {
		m_changed = true;		
		if (m_pactual!=NULL) {
			sscanf(date,"%02d/%02d/%04d %02d:%02d:%02d",&day,&month,&year,&hour,&min,&sec);
			ptr[0]=bin2bcd(year-2000U);
			ptr[1]=bin2bcd(month);
			ptr[2]=bin2bcd(day);
			ptr[3]=bin2bcd(hour);
			ptr[4]=bin2bcd(min);
			ptr[5]=0;
			memcpy(m_pactual->Date_Crea,ptr,6U);
			memcpy(m_pactual->Date_Send,ptr,6U);		
			memcpy(m_pactual->Date_Rcv,ptr,6U);
			sprintf(date,"%04d/%02d/%02d %02d:%02d",year,month,day,hour,min);
			memcpy(m_pactual->subject,date,16U);			
		}
	}
}

static void put_eol(char *cad,uint16_t tam){
	uint16_t i=tam;
		
	if (cad!=NULL) {
		while (i>0) {
//			sprintf(tmp_cad,"Char = %d",*(cad+i-1));
 //           MessageBox(0, tmp_cad, "Error", MB_ICONEXCLAMATION | MB_OK);
			if ((*(cad+i-1)<127U) && (*(cad+i-1)!=0x20)) break;						
			i--;
		}
		cad[i]=0;		
	}
}

void CData::from(char *from, bool read){
		
	if (m_actual!=NULL) {
		memcpy(from,m_actual->From,16U);
		put_eol(from,16U);
		
	} else strcpy(from,"");		
}

void CData::pfrom(char *from, bool read){
		
	if (m_pactual!=NULL) {
		memcpy(from,m_pactual->From,16U);
		put_eol(from,16U);
	} else strcpy(from,"");		
}

void CData::to(char *to, bool read) {
	if (m_actual!=NULL) {
		memcpy(to,m_actual->To,16U);
		put_eol(to,16U);
	} else strcpy(to,"");			
}

void CData::pto(char *to, bool read) {
	if (m_pactual!=NULL) {
		memcpy(to,m_pactual->To,16U);
		put_eol(to,16U);
	} else strcpy(to,"");			
}

void CData::repeater(char *repeat, bool read){
	char ptr[6];
	
	if (m_actual!=NULL) {
		memcpy(ptr,m_actual->repeater,5U);
		if (ptr[0] == 0) strcpy(repeat,"");
		else {
			ptr[5]=0;
			strcpy(repeat,ptr);
		}
	} else strcpy(repeat,"");		
}

void CData::prepeater(char *repeat, bool read){
	char ptr[6];
	
	if (m_pactual!=NULL) {
		memcpy(ptr,m_pactual->repeater,5U);
		if (ptr[0] == 0) strcpy(repeat,"");
		else {
			ptr[5]=0;
			strcpy(repeat,ptr);
		}
	} else strcpy(repeat,"");		
}

void CData::position(char *position, bool read){
	uint8_t ptr[21];
	char tmp[10];
	uint16_t lat,lon,m_intlat,m_intlon,s_intlat,s_intlon;
	float mlat,mlon,slat,slon;
	
	if (m_actual!=NULL) {
		memcpy(ptr,m_actual->Gps,20U);
		if ((ptr[0] == 'N') || (ptr[0] == 'S')) {
			ptr[20]=0;
			strncpy(tmp,(char *)ptr+1,3);tmp[3]=0;
			lat=atoi(tmp);
			strncpy(tmp,(char *)ptr+11,3);tmp[3]=0;
			lon=atoi(tmp);
			sprintf(tmp,"%c%c.%c%c%c%c",ptr[4],ptr[5],ptr[6],ptr[7],ptr[8],ptr[9]);
			mlat=atof(tmp);	
			sprintf(tmp,"%c%c.%c%c%c%c",ptr[14],ptr[15],ptr[16],ptr[17],ptr[18],ptr[19]);
			mlon=atof(tmp);
			slat=mlat-floor(mlat);
			m_intlat=(uint16_t)mlat;
			s_intlat=(uint16_t) floor(slat*60.0);
			slon=mlon-floor(mlon);
			m_intlon=(uint16_t)mlon;
			s_intlon=(uint16_t) floor(slon*60.0);
						
			sprintf(position,"%c:%3d %2d\' %2d\" / %c:%3d %2d\' %2d\"",ptr[0],lat,m_intlat,s_intlat,ptr[10],
			lon, m_intlon,s_intlon);
		} else strcpy(position,"-:-- --\' --\" / -:-- --\' --\"");
	} else strcpy(position,"-:-- --\' --\" / -:-- --\' --\"");	
	
}

void CData::pposition(char *position, bool read){
	uint8_t ptr[21];
	char tmp[10];
	uint16_t lat,lon,m_intlat,m_intlon,s_intlat,s_intlon;
	float mlat,mlon,slat,slon;
	
	if (m_pactual!=NULL) {
		memcpy(ptr,m_pactual->Gps,20U);
		if ((ptr[0] == 'N') || (ptr[0] == 'S')) {
			ptr[20]=0;
			strncpy(tmp,(char *)ptr+1,3);tmp[3]=0;
			lat=atoi(tmp);
			strncpy(tmp,(char *)ptr+11,3);tmp[3]=0;
			lon=atoi(tmp);
			sprintf(tmp,"%c%c.%c%c%c%c",ptr[4],ptr[5],ptr[6],ptr[7],ptr[8],ptr[9]);
			mlat=atof(tmp);	
			sprintf(tmp,"%c%c.%c%c%c%c",ptr[14],ptr[15],ptr[16],ptr[17],ptr[18],ptr[19]);
			mlon=atof(tmp);
			slat=mlat-floor(mlat);
			m_intlat=(uint16_t)mlat;
			s_intlat=(uint16_t) floor(slat*60.0);
			slon=mlon-floor(mlon);
			m_intlon=(uint16_t)mlon;
			s_intlon=(uint16_t) floor(slon*60.0);
						
			sprintf(position,"%c:%3d %2d\' %2d\" / %c:%3d %2d\' %2d\"",ptr[0],lat,m_intlat,s_intlat,ptr[10],
			lon, m_intlon,s_intlon);
		} else strcpy(position,"-:-- --\' --\" / -:-- --\' --\"");
	} else strcpy(position,"-:-- --\' --\" / -:-- --\' --\"");	
	
}

void CData::subject(char *subject, bool read){
	if (read) {
		if (m_actual!=NULL) {
			memcpy(subject,m_actual->subject,16U);
			put_eol(subject,16U);
		} else strcpy(subject,"");
	} else {
		m_changed = true;
		if (m_actual!=NULL) {
			strcpyfill(m_actual->subject,subject,16U);
		}
	}
}

void CData::psubject(char *subject, bool read){
	if (read) {
		if (m_pactual!=NULL) {
			memcpy(subject,m_pactual->subject,16U);
			put_eol(subject,16U);
		} else strcpy(subject,"");
	} else {
		m_changed = true;
		if (m_pactual!=NULL) {
			strcpyfill(m_pactual->subject,subject,16U);
		}
	}
}

void CData::filename(char *filenam, bool read){
	FILE *file;
	uint16_t acu;
	uint32_t hi,lo;
	
	if (read) {
		if (m_pactual!=NULL) {
			memcpy(filenam,m_pactual->filename,16U);
			put_eol(filenam,16U);
		} else strcpy(filenam,"");
	} else {
		m_changed = true;
		if (m_pactual!=NULL) {				
			strcpyfill(m_pactual->filename,getFileName(filenam),16U);
			
		if ((file=fopen(filenam,"rb")) == NULL) {
				MessageBox(0, "Error opening file picture", "Error", MB_ICONEXCLAMATION | MB_OK);				
				return;
		}
		acu=0;
		do {
			fgetc(file);			
			acu++;			
		} while (!feof(file));
		fclose(file);
		acu--;
		hi=acu/256;lo=acu%256;
		m_pactual->size=(lo<<24)+(hi<<16);
		memcpy(m_pactual->signature,m_signature+1,5U);					
		}
	}
}

void CData::text_data(char *text, bool read){
	if (read) {
		if (m_actual!=NULL) {
			memcpy(text,m_actual->text,80U);
			put_eol(text,80U);
		} else strcpy(text,"");
	} else {
		m_changed = true;
		if (m_actual!=NULL) {
			strcpyfill(m_actual->text,text,80U);
		}
	}		
}	

void CData::dump(char *buffer, uint8_t *data, uint16_t size)
{
	char ascii[17],buf[10];
	size_t i, j;
	
	strcpy(buffer,"");
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		sprintf(buf,"%02X ", ((unsigned char*)data)[i]);
		strcat(buffer,buf);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			sprintf(buf," ");
			strcat(buffer,buf);
			if ((i+1) % 16 == 0) {
				sprintf(buf,"|  %s \n", ascii);
				strcat(buffer,buf);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					sprintf(buf," ");
					strcat(buffer,buf);
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					sprintf(buf,"   ");
					strcat(buffer,buf);
				}
				sprintf(buf,"|  %s \n", ascii);
				strcat(buffer,buf);
			}
		}
	}	
	
}

void CData::remove_node(uint16_t number) {
uint16_t i;
TxtNode *tmp_node;
TxtNode *temp;


   if (number == 0) {
   	   temp = m_head;
   	   m_head = m_head->next,
   	   delete temp;
   } else {
	   tmp_node = m_head;
	   for (i=0;i<number-1;i++) {
	   	  tmp_node= tmp_node->next;
	   	  if (tmp_node==NULL) return;
	   }
	   if (tmp_node->next == NULL) return;
	   temp = tmp_node->next;
	   tmp_node->next = temp->next;
	   delete temp;
	}
	m_nodes--;
	m_changed = true;
}

void CData::remove_pnode(uint16_t number){
uint16_t i;
PctNode *tmp_node;
PctNode *temp;


   if (number == 0) {
   	   temp = m_phead;
   	   m_phead = m_phead->next,
   	   delete temp;
   } else {
	   tmp_node = m_phead;
	   for (i=0;i<number-1;i++) {
	   	  tmp_node= tmp_node->next;
	   	  if (tmp_node==NULL) return;
	   }
	   if (tmp_node->next == NULL) return;
	   temp = tmp_node->next;
	   tmp_node->next = temp->next;
	   delete temp;
	}
	m_pnodes--;
	m_changed = true;
}

uint16_t CData::num_nodes(void){
	return m_nodes;
}

uint16_t CData::num_pnodes(void){
	return m_pnodes;
}

void CData::clear_nodes(){
	TxtNode *next_node;
	TxtNode *tmp;
	
	next_node = m_head;
	while (next_node != NULL) {
		tmp = next_node;
		next_node = next_node->next;
		delete tmp;	
	}
	m_head = NULL;
	m_tail = NULL;
	m_nodes = 0;
}

void CData::clear_pnodes(void){
	PctNode *next_node;
	PctNode *tmp;
	
	next_node = m_phead;
	while (next_node != NULL) {
		tmp = next_node;
		next_node = next_node->next;
		delete tmp;	
	}
	m_phead = NULL;
	m_ptail = NULL;
	m_pnodes = 0;
}

void CData::add_node(uint8_t *data)
{
	uint8_t *buf1;
	SYSTEMTIME lt = {0};	
	    
    TxtNode *tmp = new TxtNode;
    buf1 = (uint8_t *) &(tmp->mark);
    if (data == NULL) {
    	m_changed = true;
	    memset(buf1,0x20,128U);
	    memset(buf1+128U,0x20,80U); // text blank
	    memcpy(tmp->From,m_config.callsign,16U);
	    if (m_config.Gps[0]!=0) memcpy(tmp->Gps,m_config.Gps,20U);
	    else memset(tmp->Gps,0xFF,20U); // Gps blank
		strcpyfill(tmp->To,(char *)"ALL",16U);
		strcpyfill(tmp->subject,(char *)"",16U);
		strcpyfill(tmp->text,(char *)"",80U);
		memset(tmp->fill,0x00,8U);
		tmp->mark=0x63;  // 61 enviado 63 recibido 0C es repetidor, 14, 15, 05, 70
		buf1[0x50] = 0x00;
		buf1[0x51] = 0x00;
		buf1[0x52] = (m_pos_msg)/256;
		buf1[0x53] = (m_pos_msg)%256;
		m_pos_msg+=0x50;
		buf1[2U] = m_nodes/256;
		buf1[3U] = m_nodes%256;			
	} else memcpy(buf1,data,208U);

	m_nodes++;	
    tmp->next = NULL;
	m_actual = tmp;
    if(m_head == NULL) {
        m_head = tmp;
        m_tail = tmp;
    } else {
        m_tail->next = tmp;
        m_tail = m_tail->next;
    }

}

void CData::add_pnode(uint8_t *data){
	uint8_t *buf1;	
	    
    PctNode *tmp = new PctNode;
    buf1 = (uint8_t *) &(tmp->mark);
    if (data == NULL) {
		m_changed = true;	
	    memset(buf1,0x20,128U);
        memcpy(tmp->From,m_config.callsign,16U);
	    if (m_config.Gps[0]!=0) memcpy(tmp->Gps,m_config.Gps,20U);
	    else memset(tmp->Gps,0xFF,20U); // Gps blank
		//strcpyfill(tmp->To,(char *)"ALL",16U);
		strcpyfill(tmp->subject,(char *)"",16U);
		memset(tmp->fill,0x00,8U);
	//	((tmp->mark=0x70;  //70 means get from received
		tmp->size=0;
		buf1[0] = 0x0;
		buf1[1] = 0x0;		
		buf1[2] = 0x0;		
		buf1[3] = 0x0;		
		//buf1[2] = m_pnodes/256;
		//buf1[3] = m_pnodes%256;		 
	} else {
		memcpy(buf1,data,128U);
	    memcpy(tmp_cad,tmp->filename,6U);
	    tmp_cad[6U]=0;
	    memcpy(tmp_cad,(char *)tmp->filename+6U,10U);
	    tmp_cad[6]=0;
	    //if (strlen(tmp_cad) == 6U) m_pictures=atoi(tmp_cad)+1;			
	}

    tmp->next = NULL;
	m_pactual = tmp;
    if(m_phead == NULL) {
        m_phead = tmp;
        m_ptail = tmp;
    } else {
        m_ptail->next = tmp;
        m_ptail = m_ptail->next;
    }
    m_pnodes++;	
}

void CData::get_filename(char *filen){
	
	strcpy(tmp_cad,m_basename);
	strcat(tmp_cad,m_signature);
	sprintf(filen,"%s%06d.jpg",tmp_cad,m_pictures);	
	m_pictures++;
}

bool CData::changed(void) {
	return m_changed;
}

	char buf1[81];
	char buf2[81];
	
void CData::remove_dup(void) {
	TxtNode *actual,*tmp;
	uint16_t i,j,k,count;
	FILE *file_msg;

	uint32_t pos;
	uint8_t *ptr;
	
//	getBaseName(msg_name,file_name);
	
//	strcat(msg_name,"QSOMSG.DAT");
		
	if ((file_msg=fopen(msg_name,"rb")) == NULL) {
		MessageBox(0, "Error with QSOMSG.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
		return;
	}	
	i=0;
	actual = m_head;
	while (actual!=NULL) {
		tmp = actual->next;
		j=1;
			pos = __builtin_bswap32(actual->pos);
			fseek(file_msg,pos,SEEK_SET);
			count = fread(buf1,1,80U,file_msg);
			if (count != 80) {
				MessageBox(0, "Error with QSOMSG.DAT", "Error", MB_ICONEXCLAMATION | MB_OK);
				return;
			}		
		while (tmp!=NULL) {
			pos = __builtin_bswap32(tmp->pos);
			fseek(file_msg,pos,SEEK_SET);
			count = fread(buf2,1,80U,file_msg);
			if (count != 80) {
				MessageBox(0, "Error with QSOMSG.DAT", "Error", MB_ICONEXCLAMATION | MB_OK); 
				return;
			}
			k=0;
			do {
				if (buf1[k]!=buf2[k]) break;
				k++;
			} while (k<80);
		
			if (k==80) remove_node(i+j);
			else j++;
			tmp = tmp->next;
		}
		actual=actual->next;
		i++;
	}
	
	
	fclose(file_msg);
}

void CData::remove_pdup(void) {
	PctNode *actual,*tmp;
	uint16_t i,j;
	uint32_t size1,size2;
	uint8_t *buf1;	
	    

	i=0;
	actual = m_phead;
	while (actual!=NULL) {
    	buf1 = (uint8_t *) &(actual->mark);
		memset(actual->To,0x20,16U);
		buf1[0] = 0x0;
		buf1[1] = 0x0;		
		buf1[2] = 0x0;		
		buf1[3] = 0x0;			
		tmp = actual->next;
		j=1;
		size1 = __builtin_bswap32(actual->size);
		
		while (tmp!=NULL) {
			size2 = __builtin_bswap32(tmp->size);
			if (size1==size2) remove_pnode(i+j);
			else j++;
			tmp = tmp->next;
		}
		actual=actual->next;
		i++;
	}
}
