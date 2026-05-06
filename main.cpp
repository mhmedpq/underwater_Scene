//* Name : Muhammed Awad Farag Hamed
//* ID : 232611000037
//* This program simulates an underwater scene with animated fish and bubbles using OpenGL and GLUT.
//*  The user can interact with the scene by adding more fish, generating more bubbles, changing fish colors, or resetting the scene.
//* Controls:
//*   F   – add a new fish
//*   B   – add more bubbles
//*   C   – randomise fish colours
//*   R   – reset the scene
//*   ESC – exit
//* how to compile: g++ main.cpp -o underwater -lGL -lGLU -lglut
//* run the program: ./underwater

#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <cstdio>

// ── Window / world dimensions ────────────────────────────────────────────────
static const int WIN_W = 1200; // window pixel width  (was 1200)
static const int WIN_H = 800;  // window pixel height (was 800)

// ── Timing ───────────────────────────────────────────────────────────────────
// 60 fps target (≈16 ms).  Smaller step → smoother curves.
static const int TIMER_MS = 16;
static const float DT = 0.016f; // seconds per frame

// ── Scene limits ─────────────────────────────────────────────────────────────
static const int MAX_FISH = 50;
static const int INIT_BUBBLES = 300; // scaled down from 300

static const float PI = 3.14159265f;

// ─────────────────────────────────────────────────────────────────────────────
struct Fish
{
    float x, y;
    float baseY;
    float speed;     // pixels / frame
    float direction; // +1 right, -1 left
    float size;
    float r, g, b;
    float time; // personal phase accumulator
};

struct Bubble
{
    float x, y;
    float speed;
    float radius;
};

static std::vector<Fish> fishes;
static std::vector<Bubble> bubbles;
static float globalTime = 0.0f;

// ─────────────────────────────────────────────────────────────────────────────
static float randRange(float lo, float hi)
{
    return lo + (hi - lo) * (static_cast<float>(rand()) / RAND_MAX);
}

// ── Primitive helpers ─────────────────────────────────────────────────────────
static void drawFilledCircle(float cx, float cy, float radius, int segments = 32)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; ++i)
    {
        float a = 2.0f * PI * i / segments;
        glVertex2f(cx + radius * cosf(a), cy + radius * sinf(a));
    }
    glEnd();
}

static void drawCircleOutline(float cx, float cy, float radius, int segments = 32)
{
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; ++i)
    {
        float a = 2.0f * PI * i / segments;
        glVertex2f(cx + radius * cosf(a), cy + radius * sinf(a));
    }
    glEnd();
}

// ── Fish factory ──────────────────────────────────────────────────────────────
static Fish makeFish(float x, float y)
{
    Fish f;
    f.x = x;
    f.y = y;
    f.baseY = y;
    f.speed = randRange(0.6f, 1.4f); // was 1–2; smoother at smaller canvas
    f.direction = (rand() % 2 == 0) ? 1.0f : -1.0f;
    f.size = randRange(0.6f, 1.4f); // was 0.8–1.9; scaled for 1000-wide canvas
    f.r = randRange(0.3f, 1.0f);
    f.g = randRange(0.2f, 1.0f);
    f.b = randRange(0.1f, 0.9f);
    f.time = randRange(0.0f, 2.5f * PI);
    return f;
}

void initFish()
{
    fishes.clear();

    // Preset fish – positions rescaled to 1000×600 world
    struct
    {
        float x, y, r, g, b, size;
    } presets[] = {
        {120.0f, 300.0f, 1.0f, 0.55f, 0.0f, 1.9f},
        {380.0f, 220.0f, 0.2f, 0.8f, 1.0f, 1.6f},
        {220.0f, 150.0f, 1.0f, 0.2f, 0.4f, 1.2f},
        {490.0f, 340.0f, 0.6f, 1.0f, 0.3f, 1.1f},
        {190.0f, 380.0f, 1.0f, 0.55f, 0.0f, 1.8f},
        {300.0f, 300.0f, 0.2f, 0.8f, 1.0f, 0.9f},
        {530.0f, 220.0f, 1.0f, 0.2f, 0.4f, 1.5f},
        {570.0f, 415.0f, 0.6f, 1.0f, 0.3f, 1.3f},
    };

    for (auto &p : presets)
    {
        Fish f = makeFish(p.x, p.y);
        f.r = p.r;
        f.g = p.g;
        f.b = p.b;
        f.size = p.size;
        fishes.push_back(f);
    }
}

// ── Bubble factory ────────────────────────────────────────────────────────────
static Bubble makeBubble()
{
    Bubble b;
    b.x = randRange(20.0f, WIN_W - 20.0f);
    b.y = randRange(50.0f, 100.0f);
    b.speed = randRange(0.3f, 1.3f);  // was 0.4–1.8; gentler rise
    b.radius = randRange(2.5f, 5.5f); // was 3–7; scaled for smaller canvas
    return b;
}

void initBubbles()
{
    bubbles.clear();
    for (int i = 0; i < INIT_BUBBLES; ++i)
    {
        Bubble b = makeBubble();
        b.y = randRange(50.0f, WIN_H - 15.0f); // spread through full height at start
        bubbles.push_back(b);
    }
}

// ── Rock ─────────────────────────────────────────────────────────────────────
void drawRock(float x, float y, float scale)
{
    glColor3f(0.45f, 0.42f, 0.38f);
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    glBegin(GL_POLYGON);
    glVertex2f(-30, 0);
    glVertex2f(30, 0);
    glVertex2f(45, 22);
    glVertex2f(15, 38);
    glVertex2f(-22, 26);
    glEnd();

    glColor3f(0.65f, 0.62f, 0.58f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 15);
    glVertex2f(15, 22);
    glVertex2f(8, 30);
    glVertex2f(-8, 22);
    glEnd();

    glPopMatrix();
}

// ── Background ────────────────────────────────────────────────────────────────
void drawBackground()
{
    // Ocean gradient  (deep blue bottom → bright blue top)
    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.18f, 0.42f);
    glVertex2f(0, 0);
    glColor3f(0.0f, 0.18f, 0.42f);
    glVertex2f(WIN_W, 0);
    glColor3f(0.1f, 0.55f, 0.85f);
    glVertex2f(WIN_W, WIN_H);
    glColor3f(0.1f, 0.55f, 0.85f);
    glVertex2f(0, WIN_H);
    glEnd();

    // Sandy floor (60 px tall – scaled from 80 at 800 h)
    glColor3f(0.76f, 0.70f, 0.50f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(WIN_W, 0);
    glVertex2f(WIN_W, 60);
    glVertex2f(0, 60);
    glEnd();

    // Sand ripple lines
    glColor3f(0.68f, 0.62f, 0.42f);
    for (int i = 0; i < 5; ++i)
    {
        float yy = 12.0f + i * 9.0f;
        glBegin(GL_QUADS);
        glVertex2f(0, yy);
        glVertex2f(WIN_W, yy);
        glVertex2f(WIN_W, yy + 3);
        glVertex2f(0, yy + 3);
        glEnd();
    }

    // Rocks – x-positions spread across 1000-wide canvas
    drawRock(110, 60, 1.1f);
    drawRock(300, 60, 1.0f);
    drawRock(560, 60, 1.05f);
    drawRock(800, 60, 1.1f);
    drawRock(960, 60, 0.9f);
}

// ── Fish drawing ──────────────────────────────────────────────────────────────
void drawFish(const Fish &f)
{
    glPushMatrix();
    glTranslatef(f.x, f.y, 0.0f);
    glScalef(f.direction * f.size, f.size, 1.0f);

    // Tail – oscillates with personal time at a comfortable frequency
    float tailAngle = sinf(f.time * 3.0f) * 22.0f; // was 25°; slightly tighter
    glPushMatrix();
    glTranslatef(-28.0f, 0.0f, 0.0f);
    glRotatef(tailAngle, 0.0f, 0.0f, 1.0f);
    glColor3f(f.r * 0.75f, f.g * 0.75f, f.b * 0.75f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(-22.0f, 16.0f);
    glVertex2f(-22.0f, -16.0f);
    glEnd();
    glPopMatrix();

    // Body ellipse
    glColor3f(f.r, f.g, f.b);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 24; ++i)
    {
        float a = 2.0f * PI * i / 24;
        glVertex2f(cosf(a) * 28.0f, sinf(a) * 14.0f);
    }
    glEnd();

    // Belly highlight
    glColor3f(fminf(f.r + 0.3f, 1.0f),
              fminf(f.g + 0.3f, 1.0f),
              fminf(f.b + 0.3f, 1.0f));
    glBegin(GL_POLYGON);
    for (int i = 0; i < 24; ++i)
    {
        float a = 2.0f * PI * i / 24;
        glVertex2f(cosf(a) * 16.0f, sinf(a) * 6.0f);
    }
    glEnd();

    // Dorsal fin
    glColor3f(f.r * 0.85f, f.g * 0.85f, f.b * 0.85f);
    glBegin(GL_TRIANGLES);
    glVertex2f(5.0f, 14.0f);
    glVertex2f(18.0f, 14.0f);
    glVertex2f(12.0f, 26.0f);
    glEnd();

    // Ventral fin
    glColor3f(f.r * 0.80f, f.g * 0.80f, f.b * 0.80f);
    glBegin(GL_TRIANGLES);
    glVertex2f(5.0f, -6.0f);
    glVertex2f(18.0f, -6.0f);
    glVertex2f(10.0f, -18.0f);
    glEnd();

    // Eye
    glColor3f(1.0f, 1.0f, 1.0f);
    drawFilledCircle(14.0f, 4.0f, 5.5f);
    glColor3f(0.05f, 0.05f, 0.05f);
    drawFilledCircle(15.0f, 4.0f, 2.5f);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawFilledCircle(16.0f, 5.5f, 1.0f);

    // Mouth
    glColor3f(f.r * 0.6f, f.g * 0.5f, f.b * 0.5f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(27.0f, 2.0f);
    glVertex2f(29.0f, 0.0f);
    glVertex2f(27.0f, -2.0f);
    glEnd();
    glLineWidth(1.0f);

    glPopMatrix();
}

// ── Bubbles drawing ───────────────────────────────────────────────────────────
void drawBubbles()
{
    for (const Bubble &b : bubbles)
    {
        glColor4f(0.85f, 0.95f, 1.0f, 0.18f);
        drawFilledCircle(b.x, b.y, b.radius);

        glColor4f(0.8f, 0.92f, 1.0f, 0.65f);
        glLineWidth(1.2f);
        drawCircleOutline(b.x, b.y, b.radius);
        glLineWidth(1.0f);

        glColor4f(1.0f, 1.0f, 1.0f, 0.75f);
        drawFilledCircle(b.x - b.radius * 0.35f,
                         b.y + b.radius * 0.35f,
                         b.radius * 0.22f, 8);
    }
}

// ── Update ────────────────────────────────────────────────────────────────────
void updateFish()
{
    for (Fish &f : fishes)
    {
        // Smaller time step increment = smoother sine wave
        f.time += 0.07f; // was 0.09

        f.x += f.speed * f.direction;

        // Gentle bob – amplitude 10 px on a 600-tall canvas
        f.y = f.baseY + sinf(f.time * 1.5f) * 10.0f; // was 12

        // Wrap at canvas edges
        float halfW = 30.0f * f.size;
        if (f.x > WIN_W + halfW)
        {
            f.direction = -1.0f;
            f.x = WIN_W + halfW;
        }
        else if (f.x < -halfW)
        {
            f.direction = 1.0f;
            f.x = -halfW;
        }

        // Keep fish inside vertical play-field (below water, above floor)
        const float FLOOR_Y = 80.0f;
        const float CEIL_Y = WIN_H - 20.0f;
        if (f.y < FLOOR_Y)
        {
            f.y = FLOOR_Y;
            f.baseY = FLOOR_Y + 10.0f;
        }
        if (f.y > CEIL_Y)
        {
            f.y = CEIL_Y;
            f.baseY = CEIL_Y - 10.0f;
        }
    }
}

void updateBubbles()
{
    for (Bubble &b : bubbles)
    {
        b.y += b.speed;
        // Gentle horizontal drift – lower frequency for smoother look
        b.x += sinf(globalTime * 0.9f + b.y * 0.04f) * 0.25f;

        if (b.y > WIN_H + b.radius)
            b = makeBubble();
    }
}

// ── GL callbacks ──────────────────────────────────────────────────────────────
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    drawBackground();
    drawBubbles();
    for (const Fish &f : fishes)
        drawFish(f);

    glDisable(GL_BLEND);
    glutSwapBuffers();
}

void timer(int /*value*/)
{
    globalTime += DT;
    updateFish();
    updateBubbles();
    glutPostRedisplay();
    glutTimerFunc(TIMER_MS, timer, 0);
}

void keyboard(unsigned char key, int /*x*/, int /*y*/)
{
    switch (key)
    {
    case 'f':
    case 'F':
        if ((int)fishes.size() < MAX_FISH)
        {
            float ny = randRange(90.0f, WIN_H - 40.0f);
            float nx = randRange(40.0f, WIN_W - 40.0f);
            fishes.push_back(makeFish(nx, ny));
        }
        break;

    case 'b':
    case 'B':
        for (int i = 0; i < 10; ++i)
            bubbles.push_back(makeBubble());
        break;

    case 'c':
    case 'C':
        for (Fish &f : fishes)
        {
            f.r = randRange(0.2f, 1.0f);
            f.g = randRange(0.2f, 1.0f);
            f.b = randRange(0.1f, 0.9f);
        }
        break;

    case 'r':
    case 'R':
        globalTime = 0.0f;
        initFish();
        initBubbles();
        break;

    case 27:
        exit(0);
    }
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WIN_W, 0.0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ── Entry point ───────────────────────────────────────────────────────────────
int main(int argc, char **argv)
{
    srand(static_cast<unsigned>(time(nullptr)));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("232611000037");

    glClearColor(0.0f, 0.15f, 0.35f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WIN_W, 0.0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    initFish();
    initBubbles();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(TIMER_MS, timer, 0);

    glutMainLoop();
    return 0;
}