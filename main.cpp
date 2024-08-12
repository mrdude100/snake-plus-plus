#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

static bool allowMove = false;
Color neonGreen = {8, 255, 8, 255};

int cellSize = 30;
int cellCount = 25;
int offset = 75;

double lastUpdateTime = 0;

enum GameState { MENU, PLAYING, GAME_OVER };
GameState state = MENU;

bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

bool EventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

class Snake
{
public:
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0};
    bool addSegment = false;

    void Draw()
    {
        for (unsigned int i = 0; i < body.size(); i++)
        {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize};
            DrawRectangleRounded(segment, 0.5, 6, neonGreen);
        }
    }

    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment == true)
        {
            addSegment = false;
        }
        else
        {
            body.pop_back();
        }
    }

    void Reset()
    {
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};

class Food
{
public:
    Vector2 position;
    Texture2D texture;

    Food(deque<Vector2> snakeBody)
    {
        Image image = LoadImage("Graphics/apple.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandomPos(snakeBody);
    }

    ~Food()
    {
        UnloadTexture(texture);
    }

    void Draw()
    {
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody)
    {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody))
        {
            position = GenerateRandomCell();
        }
        return position;
    }
};

class Game
{
public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;
    Sound eatSound;
    Sound wallSound;
    Music bgMusic;

    Game()
    {
        InitAudioDevice();
        eatSound = LoadSound("Sounds/eat.mp3");
        wallSound = LoadSound("Sounds/wall.mp3");
        bgMusic = LoadMusicStream("Sounds/menu.mp3");
        PlayMusicStream(bgMusic);
    }

    ~Game()
    {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        UnloadMusicStream(bgMusic);
        CloseAudioDevice();
    }

    void Draw()
    {
        food.Draw();
        snake.Draw();
    }

    void Update()
    {
        if (running)
        {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
        }
    }

    void CheckCollisionWithFood()
    {
        if (Vector2Equals(snake.body[0], food.position))
        {
            food.position = food.GenerateRandomPos(snake.body);
            snake.addSegment = true;
            score++;
            PlaySound(eatSound);
        }
    }

    void CheckCollisionWithEdges()
    {
        if (snake.body[0].x == cellCount || snake.body[0].x == -1)
        {
            GameOver();
        }
        if (snake.body[0].y == cellCount || snake.body[0].y == -1)
        {
            GameOver();
        }
    }

    void GameOver()
    {
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        running = false;
        score = 0;
        PlaySound(wallSound);
    }

    void CheckCollisionWithTail()
    {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody))
        {
            GameOver();
        }
    }
};

void DrawMenu()
{
    ClearBackground(BLACK);
    DrawText("Snake++", 20, 20, 80, neonGreen);
    DrawText("Press ENTER to Start", offset - 5, offset + cellSize * cellCount / 2, 30, neonGreen);
}

int main()
{
    cout << "Starting the game..." << endl;
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Snake++");
    SetTargetFPS(60);

    Game game = Game();

    while (WindowShouldClose() == false)
    {
        UpdateMusicStream(game.bgMusic); // Update music stream
        BeginDrawing();

        if (state == MENU)
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                state = PLAYING;
            }
            DrawMenu();
        }
        else if (state == PLAYING)
        {
            if (EventTriggered(0.2))
            {
                allowMove = true;
                game.Update();
            }

            if (IsKeyPressed(KEY_W) && game.snake.direction.y != 1 && allowMove)
            {
                game.snake.direction = {0, -1};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_S) && game.snake.direction.y != -1 && allowMove)
            {
                game.snake.direction = {0, 1};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_A) && game.snake.direction.x != 1 && allowMove)
            {
                game.snake.direction = {-1, 0};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_D) && game.snake.direction.x != -1 && allowMove)
            {
                game.snake.direction = {1, 0};
                game.running = true;
                allowMove = false;
            }

            // Drawing
            ClearBackground(BLACK);
            DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, neonGreen);
            DrawText("Snake++", offset - 5, 20, 40, neonGreen);
            DrawText(TextFormat("%i", game.score), offset - 5, offset + cellSize * cellCount + 10, 40, neonGreen);
            game.Draw();
        }
        else if (state == GAME_OVER)
        {
            // Draw game over screen
            DrawText("Game Over", offset - 5, 20, 40, neonGreen);
            DrawText(TextFormat("Score: %i", game.score), offset - 5, offset + cellSize * cellCount / 2, 20, neonGreen);
            DrawText("Press ENTER to Restart", offset - 5, offset + cellSize * cellCount / 2 + 40, 20, neonGreen);

            if (IsKeyPressed(KEY_ENTER))
            {
                game.snake.Reset();
                game.food.position = game.food.GenerateRandomPos(game.snake.body);
                state = PLAYING;
            }
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
