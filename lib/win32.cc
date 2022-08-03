#include <napi.h>
#include <windows.h>
#include <string>
#include <iostream>
#include <dwmapi.h>


HWND topHandle{0};
HWND d3dStyle{0};
HWND electronRenderWindow{0};
HWND MLMainWindow{0};


struct Process {
    int handle;
    int pid;
    std::string path;
    std::string title;
};

std::vector<Process> _windows;
std::vector<Process> _childWindows;

std::string outerPath;

// string->wstring
std::wstring get_wstring(const std::string str){
  return std::wstring(str.begin(),str.end());
}

// stirng->utf8 string
std::string toUtf8 (const std::wstring& str) {
    std::string ret;
    int len = WideCharToMultiByte (CP_UTF8, 0, str.c_str (), str.length (), NULL, 0, NULL, NULL);
    if (len > 0) {
        ret.resize (len);
        WideCharToMultiByte (CP_UTF8, 0, str.c_str (), str.length (), &ret[0], len, NULL, NULL);
    }
    return ret;
}

// get the nth argument and transfer to int64
template <typename T>
T getValueFromCallbackData (const Napi::CallbackInfo& info, unsigned handleIndex) {
    return reinterpret_cast<T> (info[handleIndex].As<Napi::Number> ().Int64Value ());
}


// get the relative information for a window
Process getWindowProcess(HWND handle) {
    DWORD pid{ 0 };
    GetWindowThreadProcessId(handle, &pid);

    HANDLE pHandle{ OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pid) };

    DWORD dwSize{ MAX_PATH };
    wchar_t exeName[MAX_PATH]{};

    QueryFullProcessImageNameW(pHandle, 0, exeName, &dwSize);

    CloseHandle(pHandle);

    auto wspath(exeName);
    auto path = toUtf8(wspath);

    // get title 
    int bufsize = GetWindowTextLengthW (handle) + 1;
    LPWSTR t = new WCHAR[bufsize];
    GetWindowTextW (handle, t, bufsize);

    std::wstring ws (t);
    std::string title = toUtf8 (ws);
    
    return {reinterpret_cast<int> (handle), static_cast<int> (pid), path,title };
}


// user defined callback function for top-level windows
BOOL CALLBACK EnumWindowProc(HWND hwnd, LPARAM lparam) {

    int length = GetWindowTextLengthA(hwnd);

    if (IsWindow(hwnd)  && length != 0) {
        auto process = getWindowProcess(hwnd);
        _windows.push_back(process);
    }
    return true;
}

// user defined callback function for child-level windows
BOOL CALLBACK EnumChildProc (HWND hwnd, LPARAM lparam) {

        auto process = getWindowProcess(hwnd);
        _childWindows.push_back(process);
    
    return true;
}

// get all handles whose windows are windows and 
Napi::Array getWindows(const Napi::CallbackInfo & info) {
  Napi::Env env{ info.Env()};

    _windows.clear();
    EnumWindows(&EnumWindowProc, NULL);

    // create an arry for js
    auto arr = Napi::Array::New(env);
    auto i = 0;
    for(auto _win:_windows){
      auto obj = Napi::Object::New(env);
      obj.Set("handle",_win.handle);
      obj.Set("path",_win.path);
      obj.Set("processId",_win.pid);
      obj.Set("title",_win.title);
       arr.Set(i++,Napi::Object::Object(env,obj));
    };
    return arr;
}

Napi::Array getChildWindows(const Napi::CallbackInfo& info){
    Napi::Env env{ info.Env()};

    auto handle{ getValueFromCallbackData<HWND> (info, 0) };
   
    EnumChildWindows(handle,&EnumChildProc,NULL);

   // create an arry for js
    auto arr = Napi::Array::New(env);
    auto i = 0;
    for(auto _win:_childWindows){
      auto obj = Napi::Object::New(env);
      obj.Set("handle",_win.handle);
      obj.Set("path",_win.path);
      obj.Set("processId",_win.pid);
      obj.Set("title",_win.title);
      arr.Set(i++,Napi::Object::Object(env,obj));
    };
    return arr;


}

// retrieve handle by path
Napi::Number getHandleByPath(const Napi::CallbackInfo& info){
  Napi::Env env{ info.Env () };

  outerPath = info[0].ToString().Utf8Value();
  
  for (Process win : _windows) {
       
        if (outerPath == win.path) {
            return Napi::Number::New(env,win.handle);
        }
    }
}


// Core function setParent
Napi::Value setParent(const Napi::CallbackInfo & info){
  
  Napi::Env env{ info.Env()};

  if (info.Length() < 2) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  };

  if (!info[0].IsNumber() || !info[1].IsNumber()|| !info[2].IsNumber()|| !info[3].IsNumber()) {
    Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
    return env.Null();
  };

   auto topBrowserHandle { getValueFromCallbackData<HWND> (info, 0) };
   auto d3DHandle { getValueFromCallbackData<HWND> (info, 1) };
   auto browserConentHandle { getValueFromCallbackData<HWND> (info, 2) };
   auto exeWindowHandle { getValueFromCallbackData<HWND> (info, 3) };

 

   auto topStyle = GetWindowLongPtrW(topBrowserHandle, GWL_STYLE);
   SetWindowLongPtrW(topBrowserHandle, GWL_STYLE, WS_CLIPCHILDREN | topStyle);

   auto d3dStyle = GetWindowLongPtrW(d3DHandle, GWL_STYLE);
   SetWindowLongPtrW(d3DHandle, GWL_STYLE, d3dStyle | WS_CLIPSIBLINGS | WS_CHILD);

   auto contentStyle = GetWindowLongPtrW(browserConentHandle, GWL_STYLE);
   SetWindowLongPtrW(browserConentHandle, GWL_STYLE, contentStyle | WS_CLIPSIBLINGS | WS_CHILD);
   
   EnableWindow(d3DHandle, FALSE);
   EnableWindow(browserConentHandle, FALSE);

    auto lStyle = GetWindowLongPtrW(exeWindowHandle, GWL_STYLE);
    lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
    SetWindowLongPtrW(exeWindowHandle, GWL_STYLE, WS_CHILD | WS_CLIPSIBLINGS |lStyle);     
    

     SetWindowPos(browserConentHandle, 0 , 0, 0, 0, 0, SWP_SHOWWINDOW);
     SetWindowPos(d3DHandle, 0, 0, 0, 0, 0, SWP_SHOWWINDOW);
    SetWindowPos(exeWindowHandle, HWND_NOTOPMOST , 200, 200, 400,400, SWP_SHOWWINDOW);      
   

    SetParent(exeWindowHandle,topBrowserHandle);



    ShowWindow(topBrowserHandle, SW_SHOW);
    UpdateWindow(topBrowserHandle);

    ShowWindow(d3DHandle, SW_SHOW);
    UpdateWindow(d3DHandle);

    ShowWindow(browserConentHandle, SW_SHOW);
    UpdateWindow(browserConentHandle);

   ShowWindow(exeWindowHandle, SW_SHOW);
   UpdateWindow(exeWindowHandle);
   

   return Napi::Number::New(env,0);
}

// control window form
Napi::Boolean showWindow (const Napi::CallbackInfo& info) {
    Napi::Env env{ info.Env () };

    auto handle{ getValueFromCallbackData<HWND> (info, 0) };
    std::string type{ info[1].As<Napi::String> () };

    DWORD flag{ 0 };

    if (type == "show")
        flag = SW_SHOW;
    else if (type == "hide")
        flag = SW_HIDE;
    else if (type == "minimize")
        flag = SW_MINIMIZE;
    else if (type == "restore")
        flag = SW_RESTORE;
    else if (type == "maximize")
        flag = SW_MAXIMIZE;

    return Napi::Boolean::New (env, ShowWindow (handle, flag));
}


// retrieve size/position info 
Napi::Object getWindowBounds (const Napi::CallbackInfo& info) {
    Napi::Env env{ info.Env () };

    auto handle{ getValueFromCallbackData<HWND> (info, 0) };

    RECT rect{};
    GetWindowRect (handle, &rect);
    // DwmGetWindowAttribute(handle, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof rect);

    Napi::Object bounds{ Napi::Object::New (env) };

    bounds.Set ("x", rect.left);
    bounds.Set ("y", rect.top);
    bounds.Set ("width", rect.right - rect.left);
    bounds.Set ("height", rect.bottom - rect.top);

    return bounds;
}

Napi::Boolean setWindowBounds (const Napi::CallbackInfo& info) {
    Napi::Env env{ info.Env () };

    auto handle{ getValueFromCallbackData<HWND> (info, 0) };
    Napi::Object bounds{ info[1].As<Napi::Object> () };

    BOOL b{ MoveWindow (handle, bounds.Get ("x").ToNumber (), bounds.Get ("y").ToNumber (),
                        bounds.Get ("width").ToNumber (), bounds.Get ("height").ToNumber (), true) };

    return Napi::Boolean::New (env, b);
}


Napi::Object Init(Napi::Env env,const Napi::Object exports){
   exports.Set(Napi::String::New(env,"getHandleByPath"),Napi::Function::New(env,getHandleByPath));
   exports.Set(Napi::String::New(env,"showWindow"),Napi::Function::New(env,showWindow));
   exports.Set(Napi::String::New(env,"getWindows"),Napi::Function::New(env,getWindows));
   exports.Set(Napi::String::New(env,"getChildWindows"),Napi::Function::New(env,getChildWindows));
   exports.Set(Napi::String::New(env,"setParent"),Napi::Function::New(env,setParent));
    exports.Set (Napi::String::New (env, "getWindowBounds"), Napi::Function::New (env, getWindowBounds));
     exports.Set (Napi::String::New (env, "setWindowBounds"), Napi::Function::New (env, setWindowBounds));
   return exports;
}

NODE_API_MODULE(addon,Init)