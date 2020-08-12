#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "wiresx.h"
#include <gdiplus.h>

/*  Declaración del procedimiento de ventana  */
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgConfigProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AboutDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

LONG GetDWORDRegKey(HKEY hKey, const std::wstring &strValueName, DWORD &nValue, DWORD nDefaultValue);
LONG GetBoolRegKey(HKEY hKey, const std::wstring &strValueName, bool &bValue, bool bDefaultValue);
LONG GetStringRegKey(HKEY hKey, const std::wstring &strValueName, std::wstring &strValue, const std::wstring &strDefaultValue);

CData *m_data; 
MainConfig tmp_config;
char m_mainfile[MAX_PATH];

#include <shlwapi.h>

BOOL ShouldUseUxThemeDll(void) 
{
    HMODULE hDll;
    DWORD dwMajorVersion = 0;

    hDll = LoadLibrary("COMCTL32.DLL");
    if(hDll != NULL) {
        DLLGETVERSIONPROC fn_DllGetVersion;
        DLLVERSIONINFO vi;

        fn_DllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hDll, "DllGetVersion");
        if(fn_DllGetVersion != NULL) {
            vi.cbSize = sizeof(DLLVERSIONINFO);
            fn_DllGetVersion(&vi);
            dwMajorVersion = vi.dwMajorVersion;
        }
        FreeLibrary(hDll);
    }

    return (dwMajorVersion >= 6);
}

ULONG_PTR EnableVisualStyles(VOID)
{
    TCHAR dir[MAX_PATH];
    ULONG_PTR ulpActivationCookie = FALSE;
    ACTCTX actCtx =
    {
        sizeof(actCtx),
        ACTCTX_FLAG_RESOURCE_NAME_VALID
            | ACTCTX_FLAG_SET_PROCESS_DEFAULT
            | ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID,
        TEXT("shell32.dll"), 0, 0, dir, (LPCTSTR)124
    };
    UINT cch = GetSystemDirectory(dir, sizeof(dir) / sizeof(*dir));
    if (cch >= sizeof(dir) / sizeof(*dir)) { return FALSE; /*shouldn't happen*/ }
    dir[cch] = TEXT('\0');
    ActivateActCtx(CreateActCtx(&actCtx), &ulpActivationCookie);
    return ulpActivationCookie;
}

Gdiplus::GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil)
{
	INITCOMMONCONTROLSEX icc;
    HWND hwnd;               /* Manipulador de ventana */
    MSG mensaje;             /* Mensajes recibidos por la aplicación */
    RECT rect;
	HKEY hKey;
	std::wstring strValueOfCallSign;
	std::wstring strValueOfGps;
	std::wstring strValueOfQuality;
	std::string strCallSign;
	std::string strGps;	
	std::string strQuality;
		
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	
//	if (ShouldUseUxThemeDll()) {
//		MessageBox(0, "Loaded correct DLL", "Information", MB_ICONINFORMATION | MB_OK);	
//	}
	
	
	LONG lRes = RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\YAESUMANv03", 0, KEY_READ, &hKey);
	bool bExistsAndSuccess (lRes == ERROR_SUCCESS);
	bool bDoesNotExistsSpecifically (lRes == ERROR_FILE_NOT_FOUND);
		
	// Initialise common controls.
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icc);
		
	if (lRes != ERROR_SUCCESS) {
		MessageBox(0, "As first program execution, Configuration will be show", "Information", MB_ICONINFORMATION | MB_OK);
		strcpy(tmp_config.callsign,"EA7EE");
		strcpy(tmp_config.Gps,"N037126800W007038500");
		strcpy(tmp_config.Quality,"LOW");		

		DialogBoxParam(hThisInstance, MAKEINTRESOURCE(IDD_DIALOG1), (HWND)0, (DLGPROC)DlgConfigProc,(LPARAM)&tmp_config);
		//MessageBox(0, tmp_config.Gps, "Error", MB_ICONEXCLAMATION | MB_OK);
		
	   if(RegCreateKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\YAESUMANv03", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
	      MessageBox(0, "Could not create the registry key", "Error", MB_ICONEXCLAMATION | MB_OK);
	   } else {
		    if(RegSetValueEx(hKey, "CallSign",0,REG_EXPAND_SZ, (LPBYTE)tmp_config.callsign, (DWORD) lstrlen(tmp_config.callsign)+1) != ERROR_SUCCESS) {
				MessageBox(0, "Could not create the registry value", "Error", MB_ICONEXCLAMATION | MB_OK);
		   } else {
				if(RegSetValueEx(hKey, "Gps",0,REG_EXPAND_SZ, (LPBYTE)tmp_config.Gps, (DWORD) lstrlen(tmp_config.Gps)+1) != ERROR_SUCCESS) {
				MessageBox(0, "Could not create the registry value", "Error", MB_ICONEXCLAMATION | MB_OK);
		   		} else {
					if(RegSetValueEx(hKey, "Quality",0,REG_EXPAND_SZ, (LPBYTE)tmp_config.Quality, (DWORD) lstrlen(tmp_config.Quality)+1) != ERROR_SUCCESS) {
					MessageBox(0, "Could not create the registry value", "Error", MB_ICONEXCLAMATION | MB_OK);
		   			}
		   		}
			}
		}
	
	   if(RegCloseKey(hKey) != ERROR_SUCCESS) MessageBox(0, "Failed to close hk key!", "Error", MB_ICONEXCLAMATION | MB_OK);		
		
	} else {
		GetStringRegKey(hKey, L"CallSign", strValueOfCallSign, L"bad");
		GetStringRegKey(hKey, L"Gps", strValueOfGps, L"bad");
		GetStringRegKey(hKey, L"Quality", strValueOfQuality, L"bad");
				
		strCallSign = std::string( strValueOfCallSign.begin(), strValueOfCallSign.end() );
		strGps = std::string( strValueOfGps.begin(), strValueOfGps.end() );
		strQuality = std::string( strValueOfQuality.begin(), strValueOfQuality.end() );
		strcpy(tmp_config.callsign,strCallSign.c_str());
		strcpy(tmp_config.Gps,strGps.c_str());
		strcpy(tmp_config.Quality,strQuality.c_str());
	}
	//MessageBox(0, tmp_config.Gps, "Error", MB_ICONEXCLAMATION | MB_OK);
	m_data = new CData(tmp_config.callsign,tmp_config.Gps,tmp_config.Quality);
   
    hwnd = CreateDialogParam(hThisInstance, "DialogoPrueba", 0, DlgProc, 0);
	int width=643;
	int height=225;

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
    
	Gdiplus::GdiplusShutdown(gdiplusToken);

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
	
	 if (m_data->num_nodes()==0) return;
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
	
	 if (m_data->num_pnodes() == 0) return;	
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

bool load_file_dat(HWND hDlg) {
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
			//MessageBox(0, "Cannot load file.", "Error", MB_ICONEXCLAMATION | MB_OK);
			return true;
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
			return false;			
		}						
	} else return true;
}


Gdiplus::Bitmap* ResizeClone(Gdiplus::Bitmap *bmp, INT width, INT height)
{
    UINT o_height = bmp->GetHeight();
    UINT o_width = bmp->GetWidth();
    INT n_width = width;
    INT n_height = height;
    double ratio = ((double)o_width) / ((double)o_height);
    if (o_width > o_height) {
        // Resize down by width
        n_height = static_cast<UINT>(((double)n_width) / ratio);
    } else {
        n_width = static_cast<UINT>(n_height * ratio);
    }
    Gdiplus::Bitmap* newBitmap = new Gdiplus::Bitmap(n_width, n_height, bmp->GetPixelFormat());
    Gdiplus::Graphics graphics(newBitmap);
    graphics.DrawImage(bmp, 0, 0, n_width, n_height);
    return newBitmap;
}


HRESULT GetGdiplusEncoderClsid(const std::wstring& format, GUID* pGuid)
{
    HRESULT hr = S_OK;
    UINT  nEncoders = 0;          // number of image encoders
    UINT  nSize = 0;              // size of the image encoder array in bytes
    std::vector<BYTE> spData;
    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;
    Gdiplus::Status status;
    bool found = false;

    if (format.empty() || !pGuid)
    {
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        memset(pGuid, 0, sizeof(*pGuid));;
        status = Gdiplus::GetImageEncodersSize(&nEncoders, &nSize);

        if ((status != Gdiplus::Ok) || (nSize == 0))
        {
            hr = E_FAIL;
        }
    }

    if (SUCCEEDED(hr))
    {

        spData.resize(nSize);
        pImageCodecInfo = (Gdiplus::ImageCodecInfo*)&spData.front();
        status = Gdiplus::GetImageEncoders(nEncoders, nSize, pImageCodecInfo);

        if (status != Gdiplus::Ok)
        {
            hr = E_FAIL;
        }
    }

    if (SUCCEEDED(hr))
    {
        for (UINT j = 0; j < nEncoders && !found; j++)
        {
            if (pImageCodecInfo[j].MimeType == format)
            {
                *pGuid = pImageCodecInfo[j].Clsid;
                found = true;
            }
        }

        hr = found ? S_OK : E_FAIL;
    }

    return hr;
}

Gdiplus::Bitmap *img;
Gdiplus::Bitmap *tmp_img;
char filen[200];

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    UINT indice;
	TxtNode *tmp_node;
	static HINSTANCE hInstance;
	HKEY hKey; 
	HWND hPicture;
	size_t len;
    wchar_t wcstring[255];
	std::string cad;
	std::wstring cadw;  
	std::wstring cadwr;  
  	HDC hdc;
  	PAINTSTRUCT ps;	
  	Gdiplus::Graphics *graphics;
	    
    switch (msg)                  /* manipulador del mensaje */
    {
        case WM_INITDIALOG: {
        	if (load_file_dat(hDlg)) {
        		// Error with loading
        		DestroyWindow(hDlg);
			}
			SendDlgItemMessage(hDlg, ID_TEXT, EM_LIMITTEXT, 80, 0L);
			SendDlgItemMessage(hDlg, ID_SUBJECT, EM_LIMITTEXT, 16, 0L);
		   	if (m_data->num_nodes()>0) SendDlgItemMessage(hDlg, ID_LISTA, LB_SETCURSEL, 0, 0);			
		   	if (m_data->num_pnodes()>0) SendDlgItemMessage(hDlg, ID_PLISTA, LB_SETCURSEL, 0, 0);			
           	SetFocus(GetDlgItem(hDlg, ID_PLISTA));           	
           	load_data(hDlg);
          	load_pdata(hDlg);

			if (m_data->num_pnodes()>0) {
				m_data->filename(filen,T_READ);
				if (strlen(filen)!=0) {
					strcpy(cadena,m_basename);
					strcat(cadena,filen);						
		           	cad = std::string(cadena);
		           	cadw = std::wstring( cad.begin(), cad.end() );
					if (img != NULL) delete img;
		            img = (Gdiplus::Bitmap *) new Gdiplus::Bitmap(cadw.c_str());
		            InvalidateRect(hDlg, NULL, FALSE);          	
	          		}
	          	}
           	}           	
           	return FALSE;

           	break;
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
		case WM_PAINT:
						{
					    RECT Rect;
						HWND hcontrol;
						RECT rectCW;
						
						if (img==NULL) return 0;
						if ((img->GetWidth()==0) || (img->GetHeight()==0)) return 0;
						
						hcontrol = GetDlgItem(hDlg,ID_GROUP);
						GetWindowRect(hcontrol, &rectCW); // child window rect in screen coordinates
						POINT ptCWPos = { rectCW.left, rectCW.top };
						ScreenToClient(hDlg, &ptCWPos); // transforming the child window pos
						                                  // from screen space to parent window space
						LONG iWidth, iHeight;
						iWidth = rectCW.right - rectCW.left;
						iHeight = rectCW.bottom - rectCW.top;
					

						
						rectCW.left = ptCWPos.x;
						rectCW.top = ptCWPos.y;
						rectCW.right = rectCW.left + iWidth;
						rectCW.bottom = rectCW.top + iHeight; // child window rect in parent window space
						
						hdc = BeginPaint( hDlg, &ps );
						Gdiplus::Graphics g( hdc );
					    GetWindowRect(hcontrol, &Rect);
					    
					    Gdiplus::SolidBrush solidBrush(Gdiplus::Color(240,240,240));
						Gdiplus::Brush *b_grey = solidBrush.Clone();

#define BISEL 25
						g.FillRectangle(b_grey, (int)rectCW.left, (int)rectCW.top+BISEL, (int)(rectCW.right-rectCW.left), (int)(rectCW.bottom-rectCW.top)-BISEL);
					
						if (img->GetWidth() > img->GetHeight()) {
							iWidth = rectCW.right - rectCW.left;
							iHeight = (img->GetHeight()*iWidth)/img->GetWidth();
							rectCW.bottom = rectCW.top + iHeight; // child window rect in parent window space
						} else {
							iHeight = rectCW.bottom - rectCW.top;
							iWidth = (img->GetWidth()*iHeight)/img->GetHeight();
							rectCW.right = rectCW.left + iWidth;	
						}											
					
						g.DrawImage(img, (int)rectCW.left, (int)rectCW.top+BISEL, (int)(rectCW.right-rectCW.left), (int)(rectCW.bottom-rectCW.top)-BISEL);
						EndPaint( hDlg, &ps );
						return TRUE;
						} 
						break;      
        case WM_COMMAND:
    		switch(LOWORD(wParam)) {
              case IDOK: 
				if (SendDlgItemMessage(hDlg, ID_TEXT, EM_GETMODIFY, 0, 0)) {
      				GetDlgItemText(hDlg, ID_TEXT, cadena, 80);
      				m_data->text_data(cadena,T_WRITE);
				}
            	if (SendDlgItemMessage(hDlg, ID_SUBJECT, EM_GETMODIFY, 0, 0)) {
      				GetDlgItemText(hDlg, ID_SUBJECT, cadena, 16);
      				m_data->subject(cadena,T_WRITE);
				}	
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
              	 if (m_data->num_nodes()>0) {
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
				}
              	 break;
              case IDPGOOGLE:
              	 if (m_data->num_pnodes()>0) {              	
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
					CLSID encId;
					GUID guidJpg = {};
					Gdiplus::EncoderParameters encoderParameters;
   					ULONG             quality;
   					Gdiplus::Status            stat;
					HRESULT   hres;					  
              	
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
					   	strcpy(cadena,ofn.lpstrFile);
						//MessageBox(0,cadena,"Info", MB_ICONEXCLAMATION | MB_OK);						
				        cad = std::string(cadena);
				        cadw = std::wstring( cad.begin(), cad.end() );
						if (img != NULL) delete img;
				        tmp_img = (Gdiplus::Bitmap *) new Gdiplus::Bitmap(cadw.c_str(),FALSE);
						if(stat != Gdiplus::Ok) {
						    MessageBox(0,"Error loading jpg file","Error", MB_ICONEXCLAMATION | MB_OK);
						    return 0;
						}				        
		   				if (!strcmp(tmp_config.Quality,"LOW")) img = ResizeClone(tmp_img,160,120);	
						else if (!strcmp(tmp_config.Quality,"MID")) img = ResizeClone(tmp_img,320,240);	 					        
				        else {
				       		MessageBox(0,"Error with Quality setting","Error", MB_ICONEXCLAMATION | MB_OK);
							return 0;
						}
										   	
						delete tmp_img;
	              	 	indice = SendDlgItemMessage(hDlg, ID_PLISTA, LB_GETCURSEL, 0, 0);
		              	m_data->add_pnode(NULL);
		              	get_date(cadena);              	 
						m_data->pdate(cadena,T_WRITE);
					   	m_data->get_filename(filen);
						cad = std::string(filen);
				        cadw = std::wstring( cad.begin(), cad.end() ); 
						hres = GetGdiplusEncoderClsid(L"image/jpeg", &guidJpg);
						if (hres == E_FAIL) MessageBox(0,"Error with encoder","Error", MB_ICONEXCLAMATION | MB_OK);
						stat = img->Save(cadw.c_str(), &guidJpg, NULL);
						if (stat != Gdiplus::Ok) {
						    MessageBox(0,"Error Saving jpg file","Error", MB_ICONEXCLAMATION | MB_OK);
						}
						m_data->filename(filen,T_WRITE);		              	
		              	get_date(cadena);		              			 			 	              	 
		 		        SetFocus(GetDlgItem(hDlg, ID_PLISTA));
		              	SendDlgItemMessage(hDlg, ID_PLISTA, LB_ADDSTRING, 0, (LPARAM)cadena);
						SendDlgItemMessage(hDlg, ID_PLISTA, LB_SETCURSEL, (WPARAM)m_data->num_pnodes()-1, 0);
						load_pdata(hDlg);
				        InvalidateRect(hDlg, NULL, FALSE);						
						}
              	 	break;  
					}
              case IDERASE:
              		if (m_data->num_nodes()>0) {
		              	indice = SendDlgItemMessage(hDlg, ID_LISTA, LB_GETCURSEL, 0, 0);
						SendDlgItemMessage(hDlg, ID_LISTA, LB_DELETESTRING, (WPARAM)indice, 0);
						m_data->remove_node(indice);
						load_data(hDlg);
				        SetFocus(GetDlgItem(hDlg, ID_LISTA));
						if (indice==0) SendDlgItemMessage(hDlg, ID_LISTA, LB_SETCURSEL, 0, 0);
						else SendDlgItemMessage(hDlg, ID_LISTA, LB_SETCURSEL, (WPARAM)indice-1, 0);
					}
              	 	break;
              case IDPERASE:
              	 	if (m_data->num_pnodes()>0) {              	
		              	indice = SendDlgItemMessage(hDlg, ID_PLISTA, LB_GETCURSEL, 0, 0);
						SendDlgItemMessage(hDlg, ID_PLISTA, LB_DELETESTRING, (WPARAM)indice, 0);
						m_data->remove_pnode(indice);		 			
						load_pdata(hDlg);
				        SetFocus(GetDlgItem(hDlg, ID_PLISTA));				 
						if (indice==0) SendDlgItemMessage(hDlg, ID_PLISTA, LB_SETCURSEL, 0, 0);
						else SendDlgItemMessage(hDlg, ID_PLISTA, LB_SETCURSEL, (WPARAM)indice-1, 0);
					}
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
						m_data->filename(filen,T_READ);
						if (strlen(filen)!=0) {
							strcpy(cadena,m_basename);
							strcat(cadena,filen);
							//MessageBox(0, cadena, "Error", MB_ICONEXCLAMATION | MB_OK);						
				           	cad = std::string(cadena);
				           	cadw = std::wstring( cad.begin(), cad.end() );
							if (img != NULL) delete img;
				            img = (Gdiplus::Bitmap *) new Gdiplus::Bitmap(cadw.c_str());
				            InvalidateRect(hDlg, NULL, FALSE);
						}
              	 
						break;
				   }
          	       break;
			case IDREMOVEDUP: {
				bool used=false;
				
				if (m_data->num_nodes()>2) {
					m_data->remove_dup();
					SendDlgItemMessage(hDlg, ID_LISTA, LB_RESETCONTENT, 0, 0);
				    m_data->reset();
					while (m_data->item_left()) {
						m_data->date(cadena,T_READ);
						SendDlgItemMessage(hDlg, ID_LISTA, LB_ADDSTRING, 0, (LPARAM)cadena);
						m_data->next();
					}
					used=true;
				}
				if (m_data->num_pnodes()>2) {
					m_data->remove_pdup();
					SendDlgItemMessage(hDlg, ID_PLISTA, LB_RESETCONTENT, 0, 0);
					m_data->preset();
					while (m_data->pitem_left()) {
						m_data->pdate(cadena,T_READ);
						SendDlgItemMessage(hDlg, ID_PLISTA, LB_ADDSTRING, 0, (LPARAM)cadena);
						m_data->pnext();
					}
					used=true;
				}
			   	if (used) {
					SendDlgItemMessage(hDlg, ID_LISTA, LB_SETCURSEL, 0, 0);			
				   	SendDlgItemMessage(hDlg, ID_PLISTA, LB_SETCURSEL, 0, 0);			
		           	SetFocus(GetDlgItem(hDlg, ID_PLISTA));           	
		           	load_data(hDlg);
		           	load_pdata(hDlg);
		    	}
		    }
			break;					 
			
			case ID_CONFIG: {
				DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), hDlg, (DLGPROC)DlgConfigProc,(LPARAM)&tmp_config);
				m_data->config(tmp_config.callsign,tmp_config.Gps,tmp_config.Quality);
		
			   if(RegCreateKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\YAESUMANv03", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
			      MessageBox(0, "Could not create the registry key", "Error", MB_ICONEXCLAMATION | MB_OK);
			   } else {
				    if(RegSetValueEx(hKey, "CallSign",0,REG_EXPAND_SZ, (LPBYTE)tmp_config.callsign, (DWORD) lstrlen(tmp_config.callsign)+1) != ERROR_SUCCESS) {
						MessageBox(0, "Could not create the registry value", "Error", MB_ICONEXCLAMATION | MB_OK);
				   } else {
						if(RegSetValueEx(hKey, "Gps",0,REG_EXPAND_SZ, (LPBYTE)tmp_config.Gps, (DWORD) lstrlen(tmp_config.Gps)+1) != ERROR_SUCCESS) {
						MessageBox(0, "Could not create the registry value", "Error", MB_ICONEXCLAMATION | MB_OK);
				   		} else {
							if(RegSetValueEx(hKey, "Quality",0,REG_EXPAND_SZ, (LPBYTE)tmp_config.Quality, (DWORD) lstrlen(tmp_config.Quality)+1) != ERROR_SUCCESS) {
							MessageBox(0, "Could not create the registry value", "Error", MB_ICONEXCLAMATION | MB_OK);				   			
				   			}
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
	int index;

    switch (msg) {
        case WM_INITDIALOG:
			SendDlgItemMessage(hDlg, ID_CALLSIGN, EM_LIMITTEXT, 16, 0L);
			SendDlgItemMessage(hDlg, ID_GPS, EM_LIMITTEXT, 21, 0L);
			SendDlgItemMessage(hDlg, ID_QUALITY, CB_ADDSTRING , 0, (LPARAM)"LOW");
			SendDlgItemMessage(hDlg, ID_QUALITY, CB_ADDSTRING , 0, (LPARAM)"MID");	        	
        	Datos = (MainConfig *)lParam;	
		   	SetDlgItemText(hDlg, ID_CALLSIGN, Datos->callsign);
		   	SetDlgItemText(hDlg, ID_GPS, Datos->Gps);
		   	if (!strcmp(Datos->Quality,"LOW")) SendDlgItemMessage(hDlg, ID_QUALITY,  CB_SETCURSEL , (WPARAM)0, (LPARAM)0); 
			else if (!strcmp(Datos->Quality,"MID")) SendDlgItemMessage(hDlg, ID_QUALITY,  CB_SETCURSEL , (WPARAM)1, (LPARAM)0); 
			else SendDlgItemMessage(hDlg, ID_QUALITY,  CB_SETCURSEL , (WPARAM)0, (LPARAM)0); 	   	
          	SetFocus(GetDlgItem(hDlg, ID_CALLSIGN));
           	return FALSE;
        case WM_COMMAND:
    		switch(LOWORD(wParam)) {
              case IDOK: 
              	 GetDlgItemText(hDlg, ID_CALLSIGN, Datos->callsign, 16);
              	 GetDlgItemText(hDlg, ID_GPS, Datos->Gps, 21);
              	 index = SendDlgItemMessage(hDlg, ID_QUALITY,  CB_GETCURSEL, (WPARAM)index, (LPARAM)0);
              	 switch(index) {
              	 	case 0: strcpy(Datos->Quality,"LOW");
              	 			break;
               	 	case 1: strcpy(Datos->Quality,"MID");
              	 			break;							                	 			
				   }
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

INT_PTR CALLBACK AboutDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
        case IDOK:
        case IDCANCEL:
        {
          EndDialog(hwndDlg, (INT_PTR) LOWORD(wParam));
          return (INT_PTR) TRUE;
        }
      }
      break;
    }

    case WM_INITDIALOG:
      return (INT_PTR) TRUE;
  }

  return (INT_PTR) FALSE;
}
