#include "windows.h"

// секция данных игры  
struct sprite {
    float x, y, width, height, rad, dx, dy, speed;
    float vy;
    bool onGround;
    HBITMAP hBitmap;

    HBITMAP* frames;
    int frameCount;
    int currentFrame;
    int frameDelay;
    int frameTick;

    bool faceLeft = false;

    DWORD lastFrameTime = 0;   // когда последний раз менялся кадр
    DWORD lastAttackTime = 0;  // когда последний раз нажимали ЛКМ
    int   attackFrame = 0;

    int HP;

} ;

DWORD CurrentTime =0;
sprite knite;//ракетка игрока
//sprite enemy;//ракетка противника
sprite ball;//шарик
sprite platform;
sprite attack;
sprite fire;
sprite Zombi;

float gravity = 5;

struct {
    int score, balls;//количество набранных очков и оставшихся "жизней"
    bool action = false;//состояние - ожидание (игрок должен нажать пробел) или игра
} game;

struct {
    HWND hWnd;//хэндл окна
    HDC device_context, context;// два контекста устройства (для буферизации)
    int width, height;//сюда сохраним размеры окна которое создаст программа
} window;

HBITMAP hBack;// хэндл для фонового изображения
HBITMAP knightRunFrames[3];
HBITMAP knightAttackFrames[3];
HBITMAP fireFrames[5];
HBITMAP ZombiFrames[8];


bool AttackOn = false;
//cекция кода
int stepAttack = 0;

void InitAnimation(sprite& object, HBITMAP* frames, int frameCount, int frameDelay)
{
    object.frames = frames;
    object.frameCount = frameCount;
    object.currentFrame = 0;
    object.frameDelay = frameDelay;
    object.frameTick = 0;
    object.hBitmap = frames[0];
}

void UpdateAnimation(sprite& object)
{
   
    if (object.frameCount <= 0) return;

    object.frameTick++;
    if (object.frameTick >= object.frameDelay)
    {
        object.frameTick = 0;
        object.currentFrame++;
        if (object.currentFrame >= object.frameCount)
            object.currentFrame = 0;

        object.hBitmap = object.frames[object.currentFrame];
    }

}

void UpdateAttackAnimation(sprite& object, bool mouseDown)
{
    if (object.frameCount <= 0) return;

    // Если ЛКМ нажата — запоминаем время и запускаем/продолжаем анимацию
    if (mouseDown)
    {
        object.lastAttackTime = CurrentTime;

        // Смена кадра раз в 150 мс
        if (CurrentTime - object.lastFrameTime >= 150)
        {
            object.lastFrameTime = CurrentTime;
            object.hBitmap = object.frames[object.attackFrame];
            object.attackFrame++;
            if (object.attackFrame >= object.frameCount)
                object.attackFrame = 0;   // либо оставить на последнем — см. ниже
        }
    }
    else
    {
        // ЛКМ отпущена: если прошло больше 500 мс — сброс на первый кадр
        if (CurrentTime - object.lastAttackTime >= 500)
        {
            object.attackFrame = 0;
            object.lastFrameTime = 0;
            object.hBitmap = object.frames[0];
        }
    }
}

void InitGame()
{
    //в этой секции загружаем спрайты с помощью функций gdi
    //пути относительные - файлы должны лежать рядом с .exe 
    //результат работы LoadImageA сохраняет в хэндлах битмапов, рисование спрайтов будет произовдиться с помощью этих хэндлов
    ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    knite.hBitmap = (HBITMAP)LoadImageA(NULL, "Ходьба_рыцарь.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	attack.hBitmap = (HBITMAP)LoadImageA(NULL, "Разрез1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    platform.hBitmap = (HBITMAP)LoadImageA(NULL, "Платформа_1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	fire.hBitmap = (HBITMAP)LoadImageA(NULL, "Магия Огня1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	Zombi.hBitmap = (HBITMAP)LoadImageA(NULL, "Зомби.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //enemy.hBitmap = (HBITMAP)LoadImageA(NULL, "racket_enemy.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "Деревня.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //------------------------------------------------------

    knite.width = 130;
    knite.height = 180;
    knite.speed = 20;//скорость перемещения ракетки
    knite.x = window.width / 2.;//ракетка посередине окна
    knite.y = window.height - knite.height;//чуть выше низа экрана - на высоту ракетки
    knite.vy = 0;
    knite.onGround = false;

    knightRunFrames[0] = (HBITMAP)LoadImageA(NULL, "Ходьба_рыцарь.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    knightRunFrames[1] = (HBITMAP)LoadImageA(NULL, "Ходьба_рыцаря_2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    knightRunFrames[2] = (HBITMAP)LoadImageA(NULL, "Ходьба_рыцаря_3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    InitAnimation(knite, knightRunFrames, 3, 5);


	attack.x = knite.x + knite.width/2;
	attack.y = knite.y - knite.height/2;
	attack.width = 130;
	attack.height = 180;
    attack.vy = 0;

    knightAttackFrames[0] = (HBITMAP)LoadImageA(NULL, "Разрез1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    knightAttackFrames[1] = (HBITMAP)LoadImageA(NULL, "Разрез2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    knightAttackFrames[2] = (HBITMAP)LoadImageA(NULL, "Разрез3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    InitAnimation(attack, knightAttackFrames, 3, 15);


	fire.width = 130;
	fire.height = 180;  
	fire.x = knite.x + knite.width;
	fire.y = knite.y - knite.height / 2;

    fireFrames[0] = (HBITMAP)LoadImageA(NULL, "Магия Огня1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    fireFrames[1] = (HBITMAP)LoadImageA(NULL, "Магия Огня2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    fireFrames[2] = (HBITMAP)LoadImageA(NULL, "Магия Огня3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    fireFrames[3] = (HBITMAP)LoadImageA(NULL, "Магия Огня4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    fireFrames[4] = (HBITMAP)LoadImageA(NULL, "Магия Огня5.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    InitAnimation(fire, fireFrames, 5, 10);

	Zombi.width = 130;
	Zombi.height = 180;
    Zombi.x = 100;
	Zombi.y = window.height - Zombi.height - 80;
    Zombi.HP = 10;

	ZombiFrames[0] = (HBITMAP)LoadImageA(NULL, "Зомби.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	ZombiFrames[1] = (HBITMAP)LoadImageA(NULL, "Зомби2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    ZombiFrames[2] = (HBITMAP)LoadImageA(NULL, "Зомби3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    ZombiFrames[3] = (HBITMAP)LoadImageA(NULL, "Зомби4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    ZombiFrames[4] = (HBITMAP)LoadImageA(NULL, "Зомби5.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    ZombiFrames[5] = (HBITMAP)LoadImageA(NULL, "Зомби6.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    ZombiFrames[6] = (HBITMAP)LoadImageA(NULL, "Зомби7.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    

    InitAnimation(Zombi, ZombiFrames, 7, 5);
    //enemy.x = knite.x;//х координату оппонета ставим в ту же точку что и игрока

    platform.width = 300;
    platform.height = 100;
    platform.x = 150;
    platform.y = 150;

    game.score = 0;
    game.balls = 9;

   
}

void ProcessSound(const char* name)//проигрывание аудиофайла в формате .wav, файл должен лежать в той же папке где и программа
{
    PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//переменная name содежрит имя файла. флаг ASYNC позволяет проигрывать звук паралельно с исполнением программы
}

void ShowScore()
{
    //поиграем шрифтами и цветами
    SetTextColor(window.context, RGB(160, 160, 160));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[32];//буфер для текста
    _itoa_s(game.score, txt, 10);//преобразование числовой переменной в текст. текст окажется в переменной txt
    TextOutA(window.context, 10, 10, "Score", 5);
    TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));

    _itoa_s(game.balls, txt, 10);
    TextOutA(window.context, 10, 100, "Balls", 5);
    TextOutA(window.context, 200, 100, (LPCSTR)txt, strlen(txt));
}

void ProcessInput()
{
    bool moving = false;

    if (GetAsyncKeyState('A'))
    {
        knite.x -= knite.speed;
        moving = true;
    }

    if (GetAsyncKeyState('D'))
    {
        knite.x += knite.speed;
        moving = true;
    }

    if (moving)
        UpdateAnimation(knite);
    else
    {
        knite.currentFrame = 0;
        knite.frameTick = 0;
        knite.hBitmap = knite.frames[0];
    }

    if (GetAsyncKeyState('A'))
        knite.faceLeft = true;
    if (GetAsyncKeyState('D'))
        knite.faceLeft = false;

    if (knite.onGround && GetAsyncKeyState('W'))
    {
        knite.vy = -20;
        knite.onGround = false;
    }

    bool mouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    AttackOn = mouseDown;

    if (mouseDown)
    {
        // если это ПЕРВОЕ нажатие (анимация не шла) — стартуем с нуля
        if (CurrentTime - attack.lastAttackTime > 500)
        {
            attack.attackFrame = 0;
            attack.lastFrameTime = CurrentTime - 150; // чтобы первый кадр показался сразу
            attack.hBitmap = attack.frames[0];
        }
    }

    // Анимация крутится/сбрасывается независимо от ЛКМ
    UpdateAttackAnimation(attack, mouseDown);

    // Позиция атаки привязана к рыцарю ВСЕГДА, пока атака видима
    // (пока нажата ЛКМ или пока не истекли 500 мс после отпускания)
    if (AttackOn || (CurrentTime - attack.lastAttackTime < 500))
    {
        attack.x = knite.faceLeft ? knite.x - knite.width * 1.5f
            : knite.x + knite.width / 2;
        attack.y = knite.y;
        attack.faceLeft = knite.faceLeft;
    }

    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {

        fire.x = knite.faceLeft ? knite.x - knite.width*2
            : knite.x + knite.width;
        fire.y = knite.y;
        fire.faceLeft = knite.faceLeft;

    }
        UpdateAnimation(fire);
}

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false, bool mirrorX = false)
{
    HBITMAP hOldbm;
    HDC hMemDC;
    BITMAP bm;

    hMemDC = CreateCompatibleDC(hDC);
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);

    if (hOldbm)
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm);

        if (alpha)
        {
            if (mirrorX)
            {
                StretchBlt(hDC, x + x1, y, -x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
            }
            else
            {
                TransparentBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));
            }
        }
        else
        {
            if (mirrorX)
            {
                StretchBlt(hDC, x + x1, y, -x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
            }
            else
            {
                StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
            }
        }

        SelectObject(hMemDC, hOldbm);
    }

    DeleteDC(hMemDC);
}

void ShowRacketAndBall()
{
    ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);//задний фон
    if (fire.currentFrame < 4)
    {
        ShowBitmap(window.context, fire.x, fire.y, fire.width, fire.height, fire.hBitmap, false, fire.faceLeft);

    }
    ShowBitmap(window.context, knite.x - knite.width / 2., knite.y, knite.width, knite.height, knite.hBitmap, false, knite.faceLeft);

    if (AttackOn || (CurrentTime - attack.lastAttackTime < 500))
    {
        ShowBitmap(window.context, attack.x, attack.y,attack.width, attack.height,attack.hBitmap, false, attack.faceLeft);
    }

    if (Zombi.HP>0)
    {

	    ShowBitmap(window.context, Zombi.x, Zombi.y, Zombi.width, Zombi.height, Zombi.hBitmap, false, Zombi.faceLeft);
    }
    //if (ball.dy < 0 && (enemy.x - knite.width / 4 > ball.x || ball.x > enemy.x + knite.width / 4))
    //{
    //    //имитируем разумность оппонента. на самом деле, компьютер никогда не проигрывает, и мы не считаем попадает ли его ракетка по шарику
    //    //вместо этого, мы всегда делаем отскок от потолка, а раектку противника двигаем - подставляем под шарик
    //    //движение будет только если шарик летит вверх, и только если шарик по оси X выходит за пределы половины длины ракетки
    //    //в этом случае, мы смешиваем координаты ракетки и шарика в пропорции 9 к 1
    //    enemy.x = ball.x * .1 + enemy.x * .9;
    //}
    
    ShowBitmap(window.context, platform.x, platform.y, platform.width, platform.height, platform.hBitmap);//задний фон

    //ShowBitmap(window.context, enemy.x - knite.width / 2, 0, knite.width, knite.height, enemy.hBitmap);//ракетка оппонента
    //ShowBitmap(window.context, ball.x - ball.rad, ball.y - ball.rad, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true);// шарик
}


void CheckKniteAttackCollision()
{
	if (AttackOn || (CurrentTime - attack.lastAttackTime < 500))
	{
		// Проверяем пересечение прямоугольников атаки и зомби
		if (attack.x < Zombi.x + Zombi.width &&
			attack.x + attack.width > Zombi.x &&
			attack.y < Zombi.y + Zombi.height &&
			attack.y + attack.height > Zombi.y)
		{

            Zombi.HP -= 1;
			// Коллизия произошла
			// Здесь можно добавить логику обработки коллизии, например, уменьшение здоровья зомби
			//ProcessSound("hit.wav"); // Проигрываем звук удара
			//OutputDebugStringA(Zombi.HP);
		}
	}
}
void LimitRacket()
{
    knite.x = max(knite.x, knite.width / 2.);//если коодината левого угла ракетки меньше нуля, присвоим ей ноль
    knite.x = min(knite.x, window.width - knite.width / 2.);//аналогично для правого угла
}

void CheckWalls()
{
    if (ball.x < ball.rad || ball.x > window.width - ball.rad)
    {
        ball.dx *= -1;
        ProcessSound("bounce.wav");
    }
}

void CheckRoof()
{
    if (ball.y < ball.rad + knite.height)
    {
        ball.dy *= -1;
        ProcessSound("bounce.wav");
    }
}

bool tail = false;

void CheckFloor()
{
    knite.y = max(knite.y, knite.width);//если коодината левого угла ракетки меньше нуля, присвоим ей ноль
    knite.y = min(knite.y, window.width - knite.width / 2.);//аналогично для правого угла
}
void CheckKnightFloorAndRoof()
{
    if (knite.y + knite.height >= window.height - 80)
    {
        knite.y = window.height - knite.height - 80;
        knite.vy = 0;
        knite.onGround = true;
    }

    if (knite.y <= 0)
    {
        knite.y = 0;
        if (knite.vy < 0)
            knite.vy = 0;
    }
}

void ProcessKnightPhysics()
{
    const float gravity = 1.0f;

    knite.vy += gravity;
    knite.y += knite.vy;

    CheckKnightFloorAndRoof();
}
void UpdateZombiAnimation()
{
	UpdateAnimation(Zombi);
	// Логика движения зомби
   
	if (Zombi.x > knite.x)
	{
		Zombi.x -= 10; // Двигаемся вправо
		Zombi.faceLeft = false;
	}
	else if (Zombi.x < knite.x)
	{
		Zombi.x += 10; // Двигаемся влево
		Zombi.faceLeft = true;
	}
    
}
void ProcessRoom()
{
    //обрабатываем стены, потолок и пол. принцип - угол падения равен углу отражения, а значит, для отскока мы можем просто инвертировать часть вектора движения шарика
    CheckWalls();
    CheckRoof();
    CheckFloor();
    UpdateZombiAnimation();
    CheckKniteAttackCollision();
}

void ProcessBall()
{
    if (game.action)
    {
        //если игра в активном режиме - перемещаем шарик
        ball.x += ball.dx * ball.speed;
        ball.y += ball.dy * ball.speed;
    }
    else
    {
        //иначе - шарик "приклеен" к ракетке
        ball.x = knite.x;
    }
}

void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);//из хэндла окна достаем хэндл контекста устройства для рисования
    window.width = r.right - r.left;//определяем размеры и сохраняем
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//второй буфер
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//привязываем окно к контексту
    GetClientRect(window.hWnd, &r);

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    
    InitWindow();//здесь инициализируем все что нужно для рисования в окне
    InitGame();//здесь инициализируем переменные игры

    //mciSendString(TEXT("play ..\\Debug\\music.mp3 repeat"), NULL, 0, NULL);
    ShowCursor(NULL);
    
    timeBeginPeriod(1);
    while (!GetAsyncKeyState(VK_ESCAPE))
    {

        CurrentTime = timeGetTime();

        ProcessKnightPhysics();

        ShowRacketAndBall();//рисуем фон, ракетку и шарик
        ShowScore();//рисуем очик и жизни
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)

        ProcessInput();//опрос клавиатуры
        LimitRacket();//проверяем, чтобы ракетка не убежала за экран
        ProcessBall();//перемещаем шарик
        ProcessRoom();//обрабатываем отскоки от стен и каретки, попадание шарика в картетку
    }

}
