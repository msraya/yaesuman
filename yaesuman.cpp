#include <windows.h>

#include "resource.h"
#include "wiresx.h"

/*  Declaración del procedimiento de ventana  */
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgConfigProc(HWND, UINT, WPARAM, LPARAM);

LONG GetDWORDRegKey(HKEY hKey, const std::wstring &strValueName, DWORD &nValue, DWORD nDefaultValue);
LONG GetBoolRegKey(HKEY hKey, const std::wstring &strValueName, bool &bValue, bool bDefaultValue);
LONG GetStringRegKey(HKEY hKey, const std::wstring &strValueName, std::wstring &strValue, const std::wstring &strDefaultValue);

CData *m_data; 
MainConfig tmp_config;
char m_mainfile[MAX_PATH];
	
int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil)
{
    HWND hwnd;               /* Manipulador de ventana */
    MSG mensaje;             /* Mensajes recibidos por la aplicación */
    RECT rect;
	HKEY hKey;
	LONG lRes = RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\YAESUMAN", 0, KEY_READ, &hKey);
	bool bExistsAndSuccess (lRes == ERROR_SUCCESS);
	bool bDoesNotExistsSpecifically (lRes == ERROR_FILE_NOT_FOUND);
	
	std::wstring strValueOfCallSign;
	std::wstring strValueOfGps;
	
	std::string strCallSign;
	std::string strGps;	
		
	if (lRes != ERROR_SUCCESS) {
		MessageBox(0, "As first program execution, Configuration will be show", "Information", MB_ICONINFORMATION | MB_OK);
		strcpy(tmp_config.callsign,"EA7EE");
		strcpy(tmp_config.Gps,"N037126800W007038500");

		DialogBoxParam(hThisInstance, MAKEINTRESOURCE(IDD_DIALOG1), (HWND)0, (DLGPROC)DlgConfigProc,(LPARAM)&tmp_config);
		//MessageBox(0, tmp_config.Gps, "Error", MB_ICONEXCLAMATION | MB_OK);
		
	   if(RegCreateKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\YAESUMAN", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
	      MessageBox(0, "Could not create the registry key", "Error", MB_ICONEXCLAMATION | MB_OK);
	   } else {
		    if(RegSetValueEx(hKey, "CallSign",0,REG_EXPAND_SZ, (LPBYTE)tmp_config.callsign, (DWORD) lstrlen(tmp_config.callsign)+1) != ERROR_SUCCESS) {
				MessageBox(0, "Could not create the registry value", "Error", MB_ICONEXCLAMATION | MB_OK);
		   } else {
				if(RegSetValueEx(hKey, "Gps",0,REG_EXPAND_SZ, (LPBYTE)tmp_config.Gps, (DWORD) lstrlen(tmp_config.Gps)+1) != ERROR_SUCCESS) {
				MessageBox(0, "Could not create the registry value", "Error", MB_ICONEXCLAMATION | MB_OK);
		   		} else {

		   		}
		   }
		}
	
	   if(RegCloseKey(hKey) != ERROR_SUCCESS) MessageBox(0, "Failed to close hk key!", "Error", MB_ICONEXCLAMATION | MB_OK);		
		
	} else {
		GetStringRegKey(hKey, L"CallSign", strValueOfCallSign, L"bad");
		GetStringRegKey(hKey, L"Gps", strValueOfGps, L"bad"); 
				
		strCallSign = std::string( strValueOfCallSign.begin(), strValueOfCallSign.end() );
		strGps = std::string( strValueOfGps.begin(), strValueOfGps.end() );
		strcpy(tmp_config.callsign,strCallSign.c_str());
		strcpy(tmp_config.Gps,strGps.c_str());
	}
	//MessageBox(0, tmp_config.Gps, "Error", MB_ICONEXCLAMATION | MB_OK);
	m_data = new CData(tmp_config.callsign,tmp_config.Gps);
   
    hwnd = CreateDialogParam(hThisInstance, "DialogoPrueba", 0, DlgProc, 0);
	int width=726;
	int height=146;

    GetClientRect(GetDesktopWindow(), &rect);
    rect.left = (rect.right/3) - (width/2);
    rect.top = (rect.bottom/3) - (height/2);
	SetWindowPos(hwnd,HWND_TOP,rect.left,rect.top,width,height,SWP_NOSIZE);

   /* Mostrar la ventana */
    ShowWindow(hwnd, SW_SHOWNORMAL);

    /* Bucle de mensajes, se ejecuta hasta que haya error o GetMessage devuelva FALSE */
    while(TRUE == GetMessage(&mensaje, NULL, 0, 0))
    {
    	
    	if(!IsDialogMessage(hwnd, &mensaje)) {
        /* Traducir mensajes de teclas virtuales a mensajes de caracteres */
        TranslateMessage(&mensaje);
        /* Enviar mensaje al procedimiento de ventana */
        DispatchMessage(&mensaje);
    	}
    }

    /* Salir con valor de retorno */
    return mensaje.wParam;
}

extern char cadena[512];
extern char m_basename[512];

static void dlg_load(HWND hDlg, UINT item) {
	   HWND hEdit = GetDlgItem (hDlg, item);
	   int index = GetWindowTextLength (hEdit);
	   SendMessageA(hEdit, EM_SETSEL, (WPARAM)0, (LPARAM)index); 
	   SendMessageA(hEdit, EM_REPLACESEL, 0, (LPARAM)cadena);	
}


static void load_data(HWND hDlg) {
	UINT indice;
	
	 indice = SendDlgItemMessage(hDlg, ID_LISTA, LB_GETCURSEL, 0, 0);
  	 m_data->go_to(indice);
  	 m_data->date(cadena,T_READ);
  	 SetDlgItemText(hDlg, ID_DATE, cadena);
//  	 dlg_load(hDlg,ID_DATE);
  	 m_data->from(cadena,T_READ);
  	 dlg_load(hDlg,ID_FROM);
  	 m_data->repeater(cadena,T_READ);
  	 dlg_load(hDlg,ID_REPEAT);
  	 m_data->to(cadena,T_READ);
  	 dlg_load(hDlg,ID_TO);		              	 
  	 m_data->position(cadena,T_READ);
  	 dlg_load(hDlg,ID_POSITION);
  	 m_data->subject(cadena,T_READ);
  	 dlg_load(hDlg,ID_SUBJECT);
  	 m_data->text_data(cadena,T_READ);
  	 dlg_load(hDlg,ID_TEXT);
	 SendDlgItemMessage(hDlg, ID_TEXT, EM_SETMODIFY, 0, 0);
	 SendDlgItemMessage(hDlg, ID_SUBJECT, EM_SETMODIFY, 0, 0);
}

static void load_pdata(HWND hDlg) {
	UINT indice;
	
	 indice = SendDlgItemMessage(hDlg, ID_PLISTA, LB_GETCURSEL, 0, 0);
  	 m_data->go_pto(indice);
  	 m_data->pdate(cadena,T_READ);
  	 dlg_load(hDlg,ID_PDATE);
  	 m_data->pfrom(cadena,T_READ);
  	 dlg_load(hDlg,ID_PFROM);
  	 m_data->prepeater(cadena,T_READ);
  	 dlg_load(hDlg,ID_PREPEAT);
  	 m_data->pto(cadena,T_READ);
  	 dlg_load(hDlg,ID_PTO);		              	 
  	 m_data->pposition(cadena,T_READ);
  	 dlg_load(hDlg,ID_PPOSITION);
}

static void get_date(char *date) {
SYSTEMTIME lt = {0};
  
GetLocalTime(&lt);
sprintf(date,"%02d/%02d/%04d %02d:%02d:%02d",lt.wDay,lt.wMonth,
		lt.wYear,lt.wHour,lt.wMinute,0);  
}

void load_file_dat(HWND hDlg) {
	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[260] = { 0 };       // if using TCHAR macros
	
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hDlg;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "DAT\0*.DAT\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
						
	if (GetOpenFileName(&ofn) == TRUE) {  
		m_data->clear_nodes();
		m_data->clear_pnodes();	  
		if (m_data->LoadFile(ofn.lpstrFile)) {
			MessageBox(0, "Cannot load file.", "Error", MB_ICONEXCLAMATION | MB_OK);
			return;
		} else {
			strcpy(m_mainfile,ofn.lpstrFile);
			m_data->reset();
			while (m_data->item_left()) {
				m_data->date(cadena,T_READ);
				SendDlgItemMessage(hDlg, ID_LISTA, LB_ADDSTRING, 0, (LPARAM)cadena);
				m_data->next();
			}			
			m_data->preset();
			while (m_data->pitem_left()) {
				m_data->pdate(cadena,T_READ);
				SendDlgItemMessage(hDlg, ID_PLISTA, LB_ADDSTRING, 0, (LPARAM)cadena);
				m_data->pnext();
			}			
		}						
	}
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    UINT indice;
	TxtNode *tmp_node;
	static HINSTANCE hInstance;
	HKEY hKey;    
    
    switch (msg)                  /* manipulador del mensaje */
    {
        case WM_INITDIALOG:
        	load_file_dat(hDlg);
			SendDlgItemMessage(hDlg, ID_TEXT, EM_LIMITTEXT, 80, 0L);
			SendDlgItemMessage(hDlg, ID_SUBJECT, EM_LIMITTEXT, 16, 0L);
		   	SendDlgItemMessage(hDlg, ID_LISTA, LB_SETCURSEL, 0, 0);			
		   	SendDlgItemMessage(hDlg, ID_PLISTA, LB_SETCURSEL, 0, 0);			
           	SetFocus(GetDlgItem(hDlg, ID_PLISTA));           	
           	load_data(hDlg);
           	load_pdata(hDlg);
           	return FALSE;
		case WM_CREATE:
			hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
			return 0;
		break;        
        case WM_CLOSE:
			  if(MessageBox(hDlg, "Are you sure? Data will be lost.", "Warning",  MB_ICONEXCLAMATION | MB_YESNO) == IDYES) {
			    DestroyWindow(hDlg);
			  }
			  return TRUE;        	
        	break;
        	
        case WM_DESTROY:
			  PostQuitMessage(0);
			  return TRUE;        	
        	break;
        
        case WM_COMMAND:
    		switch(LOWORD(wParam)) {
//    		  case ID_OPEN: 
//			  		load_file_dat(hDlg);
//					break;
              case IDOK: 
				if (m_data->SaveFile(m_mainfile)) {
					MessageBox(0, "Cannot save file.", "Error", MB_ICONEXCLAMATION | MB_OK);
				}
                 DestroyWindow(hDlg);
                 return TRUE;
                 break;
	          case ID_CANCEL: {
	          			if (m_data->changed()) {
							  if(MessageBox(hDlg, "Are you sure? Data will be lost.", "Warning",  MB_ICONEXCLAMATION | MB_YESNO) == IDYES) {
							    DestroyWindow(hDlg);
							  }
							  return TRUE;
						} else {
							DestroyWindow(hDlg);
							return TRUE;
						}
                 break;
             		}
              case IDGOOGLE:
              	 m_data->position(cadena,T_READ);
              	 if (cadena[0]!='-') {
              	 	    float lat,lon,mlat,mlon,slat,slon;
              	 	    char lat_sen,lon_sen;
              	 	    
              	 		sscanf(cadena,"%c:%f %f\' %f\" / %c:%f %f\' %f\"",&lat_sen,&lat,&mlat,&slat,&lon_sen,&lon,&mlon,&slon);
              	 		lon+=(slon/(60.0*60.0))+mlon/60.0;
              	 		lat+=(slat/(60.0*60.0))+mlat/60.0;
              	 		if (lon_sen == 'W') lon*=-1;
              	 		if (lat_sen == 'S') lat*=-1;
              	 	    sprintf(cadena,"start http://www.google.com/maps/place/%3.4f,%3.4f",lat,lon);
              	 		system(cadena);
				   }
              	 break;
              case IDPGOOGLE:
              	 m_data->pposition(cadena,T_READ);
              	 if (cadena[0]!='-') {
              	 	    float lat,lon,mlat,mlon,slat,slon;
              	 	    char lat_sen,lon_sen;
              	 	    
              	 		sscanf(cadena,"%c:%f %f\' %f\" / %c:%f %f\' %f\"",&lat_sen,&lat,&mlat,&slat,&lon_sen,&lon,&mlon,&slon);
              	 		lon+=(slon/(60.0*60.0))+mlon/60.0;
              	 		lat+=(slat/(60.0*60.0))+mlat/60.0;
              	 		if (lon_sen == 'W') lon*=-1;
              	 		if (lat_sen == 'S') lat*=-1;
              	 	    sprintf(cadena,"start http://www.google.com/maps/place/%3.4f,%3.4f",lat,lon);
              	 		system(cadena);
				   }
              	 break;              	 
              case IDINSERT:
              	 indice = SendDlgItemMessage(hDlg, ID_LISTA, LB_GETCURSEL, 0, 0);
              	 m_data->add_node(NULL);
              	 get_date(cadena);              	 
				 m_data->date(cadena,T_WRITE);
              	 get_date(cadena);				 			 	              	 
 		         SetFocus(GetDlgItem(hDlg, ID_LISTA));
              	 SendDlgItemMessage(hDlg, ID_LISTA, LB_ADDSTRING, 0, (LPARAM)cadena);
				 SendDlgItemMessage(hDlg, ID_LISTA, LB_SETCURSEL, (WPARAM)m_data->num_nodes()-1, 0);
				 load_data(hDlg);				 				    		         
              	 break;
              case IDPINSERT: {
	 				OPENFILENAME ofn;       		// common dialog box structure
					TCHAR szFile[260] = { 0 };       // if using TCHAR macros   
              	
					// Initialize OPENFILENAME
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = hDlg;
					ofn.lpstrFile = szFile;
					ofn.nMaxFile = sizeof(szFile);
					ofn.lpstrFilter = "JPEG\0*.JPG\0All\0*.*\0";
					ofn.nFilterIndex = 1;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
					
					if (GetOpenFileName(&ofn) == TRUE) {
						char filen[200];
											              	            	
	              	 	indice = SendDlgItemMessage(hDlg, ID_PLISTA, LB_GETCURSEL, 0, 0);
		              	m_data->add_pnode(NULL);
		              	get_date(cadena);              	 
						m_data->pdate(cadena,T_WRITE);
					   	m_data->get_filename(filen);
						sprintf(cadena,"copy %s %s",ofn.lpstrFile,filen);
						MessageBox(0, cadena, "Oh Oh...", MB_ICONEXCLAMATION | MB_OK);						
	              		system(cadena);
						m_data->filename(filen,T_WRITE);		              	
		              	get_date(cadena);		              			 			 	              	 
		 		        SetFocus(GetDlgItem(hDlg, ID_PLISTA));
		              	SendDlgItemMessage(hDlg, ID_PLISTA, LB_ADDSTRING, 0, (LPARAM)cadena);
						SendDlgItemMessage(hDlg, ID_PLISTA, LB_SETCURSEL, (WPARAM)m_data->num_pnodes()-1, 0);
						load_pdata(hDlg);
						}
              	 	break;  
					}
              case IDERASE:
	              	indice = SendDlgItemMessage(hDlg, ID_LISTA, LB_GETCURSEL, 0, 0);
					SendDlgItemMessage(hDlg, ID_LISTA, LB_DELETESTRING, (WPARAM)indice, 0);
					m_data->remove_node(indice);
					load_data(hDlg);
			        SetFocus(GetDlgItem(hDlg, ID_LISTA));
					if (indice==0) SendDlgItemMessage(hDlg, ID_LISTA, LB_SETCURSEL, 0, 0);
					else SendDlgItemMessage(hDlg, ID_LISTA, LB_SETCURSEL, (WPARAM)indice-1, 0);				 			 			 
              	 	break;
              case IDPERASE:
	              	indice = SendDlgItemMessage(hDlg, ID_PLISTA, LB_GETCURSEL, 0, 0);
					SendDlgItemMessage(hDlg, ID_PLISTA, LB_DELETESTRING, (WPARAM)indice, 0);
					m_data->remove_pnode(indice);		 			
					load_pdata(hDlg);
			        SetFocus(GetDlgItem(hDlg, ID_PLISTA));				 
					if (indice==0) SendDlgItemMessage(hDlg, ID_PLISTA, LB_SETCURSEL, 0, 0);
					else SendDlgItemMessage(hDlg, ID_PLISTA, LB_SETCURSEL, (WPARAM)indice-1, 0);	 			 
	              	break;              	 
              case ID_LISTA:
              	 switch(HIWORD(wParam)) {
              		case LBN_SELCHANGE:             			
              			if (SendDlgItemMessage(hDlg, ID_TEXT, EM_GETMODIFY, 0, 0)) {
              				GetDlgItemText(hDlg, ID_TEXT, cadena, 80);
              				m_data->text_data(cadena,T_WRITE);
						  }
              			if (SendDlgItemMessage(hDlg, ID_SUBJECT, EM_GETMODIFY, 0, 0)) {
              				GetDlgItemText(hDlg, ID_SUBJECT, cadena, 16);
              				m_data->subject(cadena,T_WRITE);
						  }						  
						load_data(hDlg);		              	 
						break;
				   }
          	       break;
              case ID_PLISTA:
              	 switch(HIWORD(wParam)) {
              		case LBN_SELCHANGE:             			
						load_pdata(hDlg);		              	 
						break;
				   }
          	       break;
//			case IDCHFILE: {
//	 				OPENFILENAME ofn;       		// common dialog box structure
//					TCHAR szFile[260] = { 0 };       // if using TCHAR macros
//					
//					// Initialize OPENFILENAME
//					ZeroMemory(&ofn, sizeof(ofn));
//					ofn.lStructSize = sizeof(ofn);
//					ofn.hwndOwner = hDlg;
//					ofn.lpstrFile = szFile;
//					ofn.nMaxFile = sizeof(szFile);
//					ofn.lpstrFilter = "JPEG\0*.JPG\0All\0*.*\0";
//					ofn.nFilterIndex = 1;
//					ofn.lpstrFileTitle = NULL;
//					ofn.nMaxFileTitle = 0;
//					ofn.lpstrInitialDir = NULL;
//					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
//					
//					if (GetOpenFileName(&ofn) == TRUE)
//					{
//						char filen[200];
//						
//					   	m_data->get_filename(filen);
//						sprintf(cadena,"copy %s %s",ofn.lpstrFile,filen);
//						//MessageBox(0, cadena, "Oh Oh...", MB_ICONEXCLAMATION | MB_OK);						
//	              		system(cadena);	
//						m_data->filename(filen,T_WRITE);
//					}
//				}
//				break;
			case IDREMOVEDUP: {
				
				m_data->remove_dup();
				SendDlgItemMessage(hDlg, ID_LISTA, LB_RESETCONTENT, 0, 0);
			    m_data->reset();
				while (m_data->item_left()) {
					m_data->date(cadena,T_READ);
					SendDlgItemMessage(hDlg, ID_LISTA, LB_ADDSTRING, 0, (LPARAM)cadena);
					m_data->next();
				}
				m_data->remove_pdup();
				SendDlgItemMessage(hDlg, ID_PLISTA, LB_RESETCONTENT, 0, 0);
				m_data->preset();
				while (m_data->pitem_left()) {
					m_data->pdate(cadena,T_READ);
					SendDlgItemMessage(hDlg, ID_PLISTA, LB_ADDSTRING, 0, (LPARAM)cadena);
					m_data->pnext();
				}									
			   	SendDlgItemMessage(hDlg, ID_LISTA, LB_SETCURSEL, 0, 0);			
			   	SendDlgItemMessage(hDlg, ID_PLISTA, LB_SETCURSEL, 0, 0);			
	           	SetFocus(GetDlgItem(hDlg, ID_PLISTA));           	
	           	load_data(hDlg);
	           	load_pdata(hDlg);
				break;
				}					 
			case IDSHFILE: {
				char filen[80];
				
				m_data->filename(filen,T_READ);
				if (strlen(filen)>0) {
					sprintf(cadena,"start %s%s",m_basename,filen);
					MessageBox(0, cadena, "Information", MB_ICONEXCLAMATION | MB_OK);
	              	system(cadena);						
				}		
				break;
				}
				
			case ID_CONFIG: {
				DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), hDlg, (DLGPROC)DlgConfigProc,(LPARAM)&tmp_config);
				m_data->config(tmp_config.callsign,tmp_config.Gps);
		
			   if(RegCreateKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\YAESUMAN", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
			      MessageBox(0, "Could not create the registry key", "Error", MB_ICONEXCLAMATION | MB_OK);
			   } else {
				    if(RegSetValueEx(hKey, "CallSign",0,REG_EXPAND_SZ, (LPBYTE)tmp_config.callsign, (DWORD) lstrlen(tmp_config.callsign)+1) != ERROR_SUCCESS) {
						MessageBox(0, "Could not create the registry value", "Error", MB_ICONEXCLAMATION | MB_OK);
				   } else {
						if(RegSetValueEx(hKey, "Gps",0,REG_EXPAND_SZ, (LPBYTE)tmp_config.Gps, (DWORD) lstrlen(tmp_config.Gps)+1) != ERROR_SUCCESS) {
						MessageBox(0, "Could not create the registry value", "Error", MB_ICONEXCLAMATION | MB_OK);
				   		} else {
				   			
				   		}
				   }
				}
			
			   if(RegCloseKey(hKey) != ERROR_SUCCESS) MessageBox(0, "Failed to close hk key!", "Error", MB_ICONEXCLAMATION | MB_OK);				
				break;			
				}
           }
           return TRUE;
    }
    return FALSE;
}



LONG GetDWORDRegKey(HKEY hKey, const std::wstring &strValueName, DWORD &nValue, DWORD nDefaultValue)
{
    nValue = nDefaultValue;
    DWORD dwBufferSize(sizeof(DWORD));
    DWORD nResult(0);
    LONG nError = ::RegQueryValueExW(hKey,
        strValueName.c_str(),
        0,
        NULL,
        reinterpret_cast<LPBYTE>(&nResult),
        &dwBufferSize);
    if (ERROR_SUCCESS == nError)
    {
        nValue = nResult;
    }
    return nError;
}


LONG GetBoolRegKey(HKEY hKey, const std::wstring &strValueName, bool &bValue, bool bDefaultValue)
{
    DWORD nDefValue((bDefaultValue) ? 1 : 0);
    DWORD nResult(nDefValue);
    LONG nError = GetDWORDRegKey(hKey, strValueName.c_str(), nResult, nDefValue);
    if (ERROR_SUCCESS == nError)
    {
        bValue = (nResult != 0) ? true : false;
    }
    return nError;
}


LONG GetStringRegKey(HKEY hKey, const std::wstring &strValueName, std::wstring &strValue, const std::wstring &strDefaultValue)
{
    strValue = strDefaultValue;
    WCHAR szBuffer[512];
    DWORD dwBufferSize = sizeof(szBuffer);
    ULONG nError;
    nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
    if (ERROR_SUCCESS == nError)
    {
        strValue = szBuffer;
    }
    return nError;
}

	MainConfig *Datos;

INT_PTR CALLBACK DlgConfigProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch (msg) {
        case WM_INITDIALOG:
			SendDlgItemMessage(hDlg, ID_CALLSIGN, EM_LIMITTEXT, 16, 0L);
			SendDlgItemMessage(hDlg, ID_GPS, EM_LIMITTEXT, 21, 0L);        	
        	Datos = (MainConfig *)lParam;	
		   	SetDlgItemText(hDlg, ID_CALLSIGN, Datos->callsign);
		   	SetDlgItemText(hDlg, ID_GPS, Datos->Gps);	
          	SetFocus(GetDlgItem(hDlg, ID_CALLSIGN));
           	return FALSE;
        case WM_COMMAND:
    		switch(LOWORD(wParam)) {
              case IDOK: 
              	 GetDlgItemText(hDlg, ID_CALLSIGN, Datos->callsign, 16);
              	 GetDlgItemText(hDlg, ID_GPS, Datos->Gps, 21);
                 EndDialog(hDlg, FALSE);
                 break;
              case IDCANCEL:
                 EndDialog(hDlg, FALSE);
                 break;	
           }
           return TRUE;
    }
    return FALSE;
}
