#include "stdlib.h"
#include "np_plugin.h"
#include "np_script_plugin_object.h"
#include "pl_ctrl.h"
#include <map>

WNDPROC CNPPlugin::lpOldProc = NULL;

CNPPlugin::CNPPlugin(NPP pNPInstance) :
m_pNPInstance(pNPInstance),
m_pWindowObj(NULL),
m_bInitialized(FALSE),
m_pScriptableObject(NULL)
{
	m_hWnd = NULL;
	m_pl_ctrl = NULL;
	NPN_GetValue(m_pNPInstance, NPNVWindowNPObject, &m_pWindowObj);
}

CNPPlugin::~CNPPlugin()
{
}

NPBool CNPPlugin::init(NPWindow* pNPWindow)
{
	if(pNPWindow == NULL)
		return FALSE;

	m_hWnd = (HWND)pNPWindow->window;
	if(m_hWnd == NULL)
		return FALSE;

	// subclass window so we can intercept window messages and
	// do our drawing to it
	//lpOldProc = SubclassWindow(m_hWnd, (WNDPROC)PluginWinProc);
	lpOldProc = (WNDPROC) SetWindowLongPtr( m_hWnd,
											GWLP_WNDPROC,
											(LONG_PTR)PluginWinProc );

	// associate window with our CPlugin object so we can access 
	// it in the window procedure
	SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)this);

	LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
	style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	SetWindowLong(m_hWnd, GWL_STYLE, style);

	m_Window = pNPWindow;
	m_bInitialized = TRUE;
	m_pl_ctrl = new CPlCtrl();//::CreateInstance(m_hWnd);
	//CPlCtrl::CreateInstance(m_hWnd)->RegisterCallBack(OnEvent, this);

	return TRUE;
}

void CNPPlugin::shut()
{
	// subclass it back
	//SubclassWindow(m_hWnd, lpOldProc);
	(WNDPROC) SetWindowLongPtr( m_hWnd,
								GWLP_WNDPROC,
								(LONG_PTR)lpOldProc );
	//CPlCtrl::ReleaseInstance(m_hWnd);
	if (m_pl_ctrl)
	{
		delete ((CPlCtrl *)m_pl_ctrl);
		m_pl_ctrl = NULL;
	}
	m_hWnd = NULL;
	m_bInitialized = FALSE;
}

NPBool CNPPlugin::isInitialized()
{
	return m_bInitialized;
}

short CNPPlugin::handleEvent(void* event)
{
	return 0;
}

// this will force to draw a version string in the plugin window
void CNPPlugin::showVersion()
{
	InvalidateRect(m_hWnd, NULL, TRUE);
	UpdateWindow(m_hWnd);

	if (m_Window) 
	{
		NPRect r =
		{
			(unsigned short)m_Window->y, (unsigned short)m_Window->x,
			(unsigned short)(m_Window->y + m_Window->height),
			(unsigned short)(m_Window->x + m_Window->width)
		};
		NPN_InvalidateRect(m_pNPInstance, &r);
	}
}

// this will clean the plugin window
void CNPPlugin::clear()
{
	InvalidateRect(m_hWnd, NULL, TRUE);
	UpdateWindow(m_hWnd);
}

void CNPPlugin::getVersion(char* *aVersion)
{
	const char *ua = NPN_UserAgent(m_pNPInstance);
	char*& version = *aVersion;
	version = (char*)NPN_MemAlloc(1 + strlen(ua));
	if (version)
		strcpy(version, ua);
	strcpy(version, "");
}

NPObject * CNPPlugin::GetScriptableObject()
{
	if (!m_pScriptableObject)
		m_pScriptableObject = NPN_CreateObject(m_pNPInstance, GET_NPOBJECT_CLASS(ScriptablePluginObject));

	if (m_pScriptableObject) 
		NPN_RetainObject(m_pScriptableObject);

	return m_pScriptableObject;
}

LRESULT CALLBACK CNPPlugin::PluginWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CNPPlugin *ud = (CNPPlugin*)GetWindowLong(hwnd,GWL_USERDATA);
	if(ud)
	{
		switch(message)
		{
		case WM_NULL:
			break;
		case WM_ERASEBKGND:
			RECT rect;
			GetClientRect(hwnd, &rect);
			FillRect((HDC)wParam, &rect, (HBRUSH) (COLOR_WINDOW+1));
			return 1;
		default:
			break;

		}
		if(ud->lpOldProc)
			return CallWindowProc(ud->lpOldProc,hwnd,message,wParam,lParam);
	}
		return DefWindowProc(hwnd, message, wParam, lParam);
}

void CNPPlugin::OnEvent(void *pUser,UINT nType, int args[],UINT argCount)
{
	CNPPlugin *pThis = reinterpret_cast<CNPPlugin*>(pUser);
	if(pThis)
		pThis->DefaultInvoke(nType,args,argCount);
}

void CNPPlugin::DefaultInvoke(UINT nType, int args[],UINT argCount)
{
	ScriptablePluginObject *pScriptablePluginObj = dynamic_cast<ScriptablePluginObject *>((ScriptablePluginObjectBase *)m_pScriptableObject);
	if (pScriptablePluginObj)
	{
		NPVariant result;
		switch(nType)
		{
		case CALLBACK_PTZCTL:
		{
			if(args[2]  <= 0) 
				return;
			NPVariant NPN_args[4];
			INT32_TO_NPVARIANT(	args[0],	NPN_args[0]);
			INT32_TO_NPVARIANT(	args[1],	NPN_args[1]);
			STRINGZ_TO_NPVARIANT((char*)args[2], NPN_args[2]);
			INT32_TO_NPVARIANT(	args[3],	NPN_args[3]);
			pScriptablePluginObj->InvokeDefault(NPN_args,argCount,&result);
			break;
		}
		case CALLBACK_ONSTATE:
		{
			if(args[2] <= 0) 
				return;

			NPVariant NPN_args[3];
			INT32_TO_NPVARIANT(	args[0],	NPN_args[0]);
			INT32_TO_NPVARIANT(	args[1],	NPN_args[1]);
			STRINGZ_TO_NPVARIANT((char*)args[2], NPN_args[2]);
			pScriptablePluginObj->InvokeDefault(NPN_args, argCount, &result);
			break;
		}
		case CALLBACK_ONVOD:
		{
			NPVariant NPN_args[2];
			INT32_TO_NPVARIANT(	args[0],	NPN_args[0]);
			INT32_TO_NPVARIANT(	args[1], NPN_args[1]);
			pScriptablePluginObj->InvokeDefault(NPN_args, argCount, &result);
			break;
		}
		default:
			break;
		}
	}
}

bool CNPPlugin::SetWorkModel(char *js_workmodel,NPVariant *result)
{
	//if(CPlCtrl::CreateInstance(m_hWnd)->InitDisPlay(m_hWnd,js_workmodel))
	if(((CPlCtrl *)m_pl_ctrl)->InitDisPlay(m_hWnd,js_workmodel))
		SetRetValue("{\"rst\":0}",result);
	else
		SetRetValue("{\"rst\":1}",result);

	return true;
}

bool CNPPlugin::Play(char *js_playInfo,NPVariant *result)
{
	//CPlCtrl::CreateInstance(m_hWnd)->RegisterCallBack(OnEvent, this);
	((CPlCtrl *)m_pl_ctrl)->RegisterCallBack(OnEvent, this);
	//if(CPlCtrl::CreateInstance(m_hWnd)->Play(js_playInfo))
	if (((CPlCtrl *)m_pl_ctrl)->Play(js_playInfo))
		SetRetValue("{\"rst\":0}",result);
	else	
		SetRetValue("{\"rst\":1}",result);
	return true;
}

bool CNPPlugin::ChangeLayout(char *js_layout,NPVariant *result)
{
	//if(CPlCtrl::CreateInstance(m_hWnd)->SetLayout(js_layout))
	if (((CPlCtrl *)m_pl_ctrl)->SetLayout(js_layout))
		SetRetValue("{\"rst\":0}",result);

	return true;
}

bool CNPPlugin::ChangePath(char *js_path,NPVariant *result)
{
	//if(CPlCtrl::CreateInstance(m_hWnd)->SetPath(js_path))
	if (((CPlCtrl *)m_pl_ctrl)->SetPath(js_path))
		SetRetValue("{\"rst\":0}",result);

	return true;
}

bool CNPPlugin::SetLayout()
{
	//CPlCtrl::CreateInstance(m_hWnd)->SetLayout();
	((CPlCtrl *)m_pl_ctrl)->SetLayout();
	return true;
}

bool CNPPlugin::StopAllPlay(NPVariant *result)
{
	//if(CPlCtrl::CreateInstance(m_hWnd)->StopAll())
	if (((CPlCtrl *)m_pl_ctrl)->StopAll())
		SetRetValue("{\"rst\":0}",result);
	else
		SetRetValue("{\"rst\":101}",result);

	return true;
}

bool CNPPlugin::VodPlayJump(char *js_time,NPVariant *result)
{
	//if(CPlCtrl::CreateInstance(m_hWnd)->VodStreamJump(js_time))
	if (((CPlCtrl *)m_pl_ctrl)->VodStreamJump(js_time))
		SetRetValue("{\"rst\":0}",result);
	else
		SetRetValue("{\"rst\":4}",result);

	return true;
}

bool CNPPlugin::GetWndParm(int nType,NPVariant *result)
{
	char info[1024] = {0};
	//if(CPlCtrl::CreateInstance(m_hWnd)->GetWndParm(info,nType))
	if (((CPlCtrl *)m_pl_ctrl)->GetWndParm(info,nType))
		SetRetValue(info,result);

	return true;
}

bool CNPPlugin::SleepPlayer(bool bSleep,NPVariant *result)
{
	//CPlCtrl::CreateInstance(m_hWnd)->SleepPlayer(bSleep);
	((CPlCtrl *)m_pl_ctrl)->SleepPlayer(bSleep);
	SetRetValue("{\"rst\":0}",result);
	return true;
}

bool CNPPlugin::SetRetValue(char *psz_ret,NPVariant *result)
{
	int len = strlen(psz_ret);
	char *tmp = (char*)NPN_MemAlloc(len+1);
	strncpy(tmp,psz_ret,len);
	tmp[len] = '\0';
	STRINGZ_TO_NPVARIANT(tmp,*result);

	return true;
}