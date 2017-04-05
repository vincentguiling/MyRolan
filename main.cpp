#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
#define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <string>
#include <stdio.h>
#include <pthread.h>
#include <shellapi.h>
#include <windowsx.h>

using namespace std;
/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("MyRolan");

#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))

typedef struct item {
    string name;
    string path;
    string icon_path;
    string arguments;
} item;
typedef struct group {
    string name;
    item* items;
    int item_num;
} group;

int WIDTH=440;
int HEIGHT=600;
int GROUP_WIDTH=100;
int GROUP_HEIGHT=40;
int ITEM_WIDTH=100;
int ITEM_HEIGHT=40;
#define BKCOLOR    RGB(240,240,240)
#define COMMON_ITEM_BKCOLOR    RGB(220,220,220)
#define ACTIVE_ITEM_BKCOLOR    RGB(200,200,200)


group* groups;
int group_num;
int current_group=0;
int current_item=0;
int current_top=0;
int item_columns=3;



int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nCmdShow)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
               WS_EX_TOOLWINDOW,                   /* Extended possibilites for variation */
               szClassName,         /* Classname */
               _T("MyRolan"),       /* Title Text */
               WS_EX_TOOLWINDOW, /* default window  WS_OVERLAPPEDWINDOW*/
               CW_USEDEFAULT,       /* Windows decides the position */
               CW_USEDEFAULT,       /* where the window ends up on the screen */
               WIDTH,                 /* The programs width */
               HEIGHT,                 /* and height in pixels */
               HWND_DESKTOP,        /* The window is a child-window to desktop */
               NULL,                /* No menu */
               hThisInstance,       /* Program Instance handler */
               NULL                 /* No Window Creation data */
           );
    DWORD dwExStyle = GetWindowLong(hwnd, GWL_STYLE);
    //dwExStyle &= ~(WS_VISIBLE);
    //dwExStyle |= WS_EX_TOOLWINDOW;
    //dwExStyle &= ~(WS_EX_APPWINDOW);
    //dwExStyle |= WS_POPUP;
    dwExStyle &=~WS_CAPTION;
    SetWindowLong(hwnd, GWL_STYLE, dwExStyle);

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}
void remove_nextline(char* buf) {
    int l=strlen(buf);
    buf[l-1]='\0';
}
int load_data(HWND hwnd) {
    FILE* f=fopen("data.txt","r");
    if(f==NULL) {
        MessageBox(hwnd,"Data File Not Exist.","Error!",0);
        return -1;
    }
    char buf[500];
    fgets(buf,500,f);
    sscanf(buf,"%d",&group_num);
    groups=new group[group_num];
    for(int i=0; i<group_num; i++) {
        fgets(buf,500,f);
        remove_nextline(buf);
        groups[i].name=buf;
        fgets(buf,500,f);
        sscanf(buf,"%d",&groups[i].item_num);
        groups[i].items=new item[groups[i].item_num];
        for(int j=0; j<groups[i].item_num; j++) {
            fgets(buf,500,f);
            remove_nextline(buf);
            buf[12]='\0';
            groups[i].items[j].name=buf;
            fgets(buf,500,f);
            remove_nextline(buf);
            groups[i].items[j].path=buf;
            fgets(buf,500,f);
            remove_nextline(buf);
            groups[i].items[j].icon_path=buf;
            fgets(buf,500,f);
            remove_nextline(buf);
            groups[i].items[j].arguments=buf;
        }
    }
    return 0;
}
long get_rect_width(RECT rect) {
    return rect.right-rect.left;
}

long get_rect_height(RECT rect) {
    return rect.bottom-rect.top;
}
long get_client_width(HWND hwnd) {
    RECT rect;
    GetClientRect (hwnd, &rect) ;
    return get_rect_width(rect);
}

long get_client_height(HWND hwnd) {
    RECT rect;
    GetClientRect (hwnd, &rect) ;
    return get_rect_height(rect);
}
int xy_to_num(int num,int columns,int width,int height,int x,int y) {
    int c=x/width;
    if(c>=columns)return -1;
    int r=y/height;
    int n=columns*r+c;
    if(n>=num)return -1;
    return n;
}
POINT get_client_mouse(HWND hwnd){
    POINT mouse;
    GetCursorPos(&mouse);//��ȡ������Ļ����
    ScreenToClient(hwnd,&mouse);//ת��Ϊ��������
    return mouse;
}
int get_current_position(HWND hwnd,int& is_group) {
    POINT mouse=get_client_mouse(hwnd);
    //printf("%d %d\n",mouse.x,mouse.y);
    if(mouse.x<0||mouse.x>WIDTH||mouse.y<0||mouse.y>HEIGHT)return -1;
    if(mouse.x<=GROUP_WIDTH) {
        is_group=1;
        int n=xy_to_num(group_num,1,GROUP_WIDTH,GROUP_HEIGHT,mouse.x,mouse.y);
        return n;
    }
    is_group=0;
    int num=groups[current_group].item_num;
    int n=xy_to_num(num,item_columns,ITEM_WIDTH,ITEM_HEIGHT,mouse.x-GROUP_WIDTH,mouse.y-current_top);
    return n;
}
int update_current_group_and_item(HWND hwnd) {
    int current_group_bak=current_group;
    int current_item_bak=current_item;
    int is_group=1;
    int n=get_current_position(hwnd,is_group);
    //printf("%d\n",n);
    if(n<0)return 0;
    if(is_group) {
        current_group=n;
        current_item=0;
        current_top=0;
    }
    else current_item=n;
    return (current_group_bak!=current_group)||(current_item_bak!=current_item)?1:0;
}
void paint_rolan(HWND hwnd,HDC hdc) {
    static HBRUSH  bk_brush =CreateSolidBrush (BKCOLOR);
    static HBRUSH  common_item_brush =CreateSolidBrush (COMMON_ITEM_BKCOLOR);
    static HBRUSH  active_item_brush =CreateSolidBrush (ACTIVE_ITEM_BKCOLOR);
    static HFONT hFont;
    LOGFONT lf;
    lf.lfHeight         = 15;
        lf.lfWidth          = 0 ;
        lf.lfEscapement     = 0 ;
        lf.lfOrientation      = 0 ;
        lf.lfWeight         = 5;
        lf.lfItalic           = 0 ;
        lf.lfUnderline       = 0 ;
        lf.lfStrikeOut       = 0 ;
        lf.lfCharSet        = DEFAULT_CHARSET ;
        lf.lfOutPrecision    = 0 ;
        lf.lfClipPrecision    = 0 ;
        lf.lfQuality         = 0 ;
        lf.lfPitchAndFamily  = 0 ;
        lstrcpy (lf.lfFaceName, _T("Arial") );

        hFont = CreateFontIndirect (&lf) ;
        SelectFont(hdc,hFont);
        SelectObject (hdc, GetStockObject (NULL_PEN)) ;

    SelectObject (hdc,bk_brush);
    Rectangle(hdc,0,0,WIDTH,HEIGHT);
    SelectObject (hdc,common_item_brush);
    SetBkColor(hdc,COMMON_ITEM_BKCOLOR);//�������ֱ���ɫ
    for(int i=0; i<group_num; i++) {

        Rectangle(hdc,0,i*GROUP_HEIGHT-1,GROUP_WIDTH,(i+1)*GROUP_HEIGHT);
        TextOut(hdc,30,i*GROUP_HEIGHT+10,&groups[i].name[0],groups[i].name.length());
    }
    int item_num=groups[current_group].item_num;
    item* items=groups[current_group].items;
    int item_count=0;
    while(item_count<item_num) {
        int left=GROUP_WIDTH+(item_count%item_columns)*ITEM_WIDTH;
        int top=current_top+item_count/item_columns*ITEM_HEIGHT;
        Rectangle(hdc,left-1,top-1,left+ITEM_WIDTH,top+ITEM_HEIGHT);
        TextOut(hdc,left+5,top+10,&items[item_count].name[0],items[item_count].name.length());
        item_count++;
    }
    SelectObject (hdc,active_item_brush);
    SetBkColor(hdc,ACTIVE_ITEM_BKCOLOR);//�������ֱ���ɫ
    Rectangle(hdc,0,current_group*GROUP_HEIGHT-1,GROUP_WIDTH,(current_group+1)*GROUP_HEIGHT);
    TextOut(hdc,30,current_group*GROUP_HEIGHT+10,&groups[current_group].name[0],groups[current_group].name.length());
    int left=GROUP_WIDTH+(current_item%item_columns)*ITEM_WIDTH;
    int top=current_top+current_item/item_columns*ITEM_HEIGHT;
    Rectangle(hdc,left-1,top-1,left+ITEM_WIDTH,top+ITEM_HEIGHT);
    TextOut(hdc,left+5,top+10,&items[current_item].name[0],items[current_item].name.length());
    //draw_line(hdc,i,0,i,N-1);
}
/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//    static char temp[100];
    static HDC hdc;
    static PAINTSTRUCT ps ;
    static POINT mouse;
    static bool is_hide;
    static int screen_height;
    static bool not_program_move;
    switch (message) {                /* handle the messages */
    case WM_CREATE: {
        not_program_move=true;
        is_hide=false;
        screen_height=GetSystemMetrics(SM_CYSCREEN);
        load_data(hwnd);
        current_group=0;
        current_item=0;
        InvalidateRect(hwnd,NULL,true);
        SetTimer(hwnd,0,100,NULL);
        //SetCapture (hwnd) ;

    }
    break;
    case WM_MOVE: {
        RECT rect;
        GetWindowRect(hwnd,&rect);
        if(not_program_move&&rect.left<=0&&rect.bottom>=screen_height){
            //ShowWindow(hwnd,SW_HIDE);
            not_program_move=false;
            MoveWindow(hwnd,1-WIDTH,1-HEIGHT, WIDTH, HEIGHT, FALSE);// screen_height-5
            not_program_move=true;
        }
        is_hide=true;
    }
    break;
    case WM_KEYDOWN: { //ʶ�𰴼�
    }
    break;
    case WM_PAINT: {
        hdc=BeginPaint (hwnd,&ps) ;
        paint_rolan(hwnd,hdc);
        //TextOut(hdc,50,50,temp,strlen(temp));
        EndPaint(hwnd,&ps);
    }
    break;
    case WM_TIMER:{
        RECT rect;
        GetClientRect(hwnd,&rect);
        //SetWindowPos(hwnd,HWND_TOP,rect.left,rect.top,WIDTH,HEIGHT,SWP_NOMOVE);
        SetWindowPos(hwnd, HWND_TOPMOST, 1, 1, 1, 1,SWP_NOSIZE|SWP_NOMOVE);
    }break;
    case WM_MOUSEMOVE: {
        //sprintf(temp,"%d %d",mouse.x,mouse.y);
        //InvalidateRect(hwnd,NULL,true);
        if(is_hide){

            //get_client_mouse(hwnd);
        //if(mouse.x<=0&&mouse.y>=screent_height){
            //ShowWindow(hwnd,SW_SHOW);
            not_program_move=false;
            MoveWindow(hwnd,0, 0, WIDTH, HEIGHT, FALSE);//screen_height-HEIGHT
        //while(true)printf("is hide\n");
            is_hide=false;
        //}
        }else{
        mouse=get_client_mouse(hwnd);
        if(mouse.x>=WIDTH-30||mouse.y<0||mouse.x<0||mouse.y>HEIGHT-30){
        //printf("mouse leave\n");
        //not_program_move=false;
        not_program_move=true;
        MoveWindow(hwnd,1-WIDTH,1-HEIGHT, WIDTH, HEIGHT, FALSE);// screen_height-5
        is_hide=true;
        }
        if(update_current_group_and_item(hwnd))
            InvalidateRect(hwnd,NULL,true);
        }
    }
    break;
    /*case WM_NCHITTEST:{
        static POINT mouse;
        GetCursorPos(&mouse);//��ȡ������Ļ����
        sprintf(temp,"%d %d",mouse.x,mouse.y);
        InvalidateRect(hwnd,NULL,true);
    }
    break;*/
    case WM_MOUSEWHEEL: { //0x020A
        int flag=0;
        if( (INT)wParam > 0 ) {
            int temp=current_top-ITEM_HEIGHT;
            if(((groups[current_group].item_num-1)/item_columns+1)*ITEM_HEIGHT+temp>0) {
                current_top=temp;
                flag=1;
            }
        }
        else {
            if(current_top<0) {//+ITEM_HEIGHT<get_client_height(hwnd)
                current_top+=ITEM_HEIGHT;
                flag=1;
            }
        }
        if(flag)
            InvalidateRect( hwnd, NULL, TRUE );
    }
    break;
    case WM_LBUTTONUP: {
        int t;
        if(get_current_position(hwnd,t)!=-1) {
            char* path= &groups[current_group].items[current_item].path[0];
            char* arguments= &groups[current_group].items[current_item].arguments[0];
            ShellExecute(NULL, "open",path,arguments, NULL, SW_SHOWNORMAL);
            InvalidateRect(hwnd,NULL,true);
        }
    }
    break;
    case WM_DESTROY:
        //ReleaseCapture () ;
        KillTimer(hwnd,0);
        PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
        break;
    default:                      /* for messages that we don't deal with */
        return DefWindowProc (hwnd, message, wParam, lParam);
    }
    return 0;
}