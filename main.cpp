#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

const char V_SHADER_PATH[] = "shaders/vertex.glsl",
           F_SHADER_PATH[] = "shaders/fragment.glsl";

const int WINDOW_WIDTH = 640,
          WINDOW_HEIGHT = 480;

float BG_RED = 0.1922f,
      BG_BLUE = 0.549f,
      BG_GREEN = 0.9059f,
      BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;
          
float object1_x = 0.0f;
float object1_y = 0.0f;
float object2_x = 0.0f;
float object2_y = 0.0f;
float object2_rotation = 0.0f;
float object2_scale = 1.0f;
float background_time = 0.0f;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix,
          g_model_matrix,
          g_projection_matrix;


/**
* Author: [Pablo O'Hop']
* Assignment: Simple 2D Scene
* Date due: 2024-02-17, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

GLuint loadTexture(const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        exit(1);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SDL_FreeSurface(surface);
    return textureID;
}

GLuint textureID1, textureID2;

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Transformation Exercise",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix       = glm::mat4(1.0f);
    g_model_matrix      = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    IMG_Init(IMG_INIT_PNG);
    textureID1 = loadTexture("/Users/pabloohop/Desktop/tatsuya.jpeg");
    textureID2 = loadTexture("/Users/pabloohop/Desktop/Maya.jpeg");
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_game_is_running = false;
        }
    }
}

void update(float delta_time)
{
    // Object 1 Movement (Diagonal)
    object1_x += 0.01f * delta_time;
    object1_y += 0.01f * delta_time;
    if (object1_x > 5.0f) object1_x = -5.0f;
    if (object1_y > 3.75f) object1_y = -3.75f;

    // Object 2 Movement (Diagonal) and Rotation
    object2_x += 0.02f * delta_time;
    object2_y += 0.02f * delta_time;
    object2_rotation += 45.0f * delta_time; // Rotate 45 degrees per second
    object2_scale = 0.5f + 0.5f * sinf(SDL_GetTicks() / 1000.0f); // Scale change

    if (object2_x > 5.0f) object2_x = -5.0f;
    if (object2_y > 3.75f) object2_y = -3.75f;

    // Background color change
    background_time += delta_time;
    if (background_time >= 2.0f) {
        BG_RED = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        BG_BLUE = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        BG_GREEN = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
        background_time = 0.0f;
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Object 1
    g_model_matrix = glm::mat4(1.0f);
    g_model_matrix = glm::translate(g_model_matrix, glm::vec3(object1_x, object1_y, 0.0f));
    g_shader_program.set_model_matrix(g_model_matrix);

    glBindTexture(GL_TEXTURE_2D, textureID1);

    float vertices[] = {
         0.5f, -0.5f, 1.0f, 0.0f,
         0.0f,  0.5f, 0.5f, 1.0f,
        -0.5f, -0.5f, 0.0f, 0.0f
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 4 * sizeof(float), vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 4 * sizeof(float), vertices + 2);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Object 2
    g_model_matrix = glm::mat4(1.0f);
    g_model_matrix = glm::translate(g_model_matrix, glm::vec3(object2_x, object2_y, 0.0f));
    g_model_matrix = glm::rotate(g_model_matrix, glm::radians(object2_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    g_model_matrix = glm::scale(g_model_matrix, glm::vec3(object2_scale, object2_scale, 1.0f));
    g_shader_program.set_model_matrix(g_model_matrix);

    glBindTexture(GL_TEXTURE_2D, textureID2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() {
    glDeleteTextures(1, &textureID1);
    glDeleteTextures(1, &textureID2);
    SDL_Quit();
}

int main(int argc, char* argv[])
{
    initialise();

    float last_frame_ticks = 0.0f;
    while (g_game_is_running)
    {
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float delta_time = ticks - last_frame_ticks;
        last_frame_ticks = ticks;

        process_input();
        update(delta_time);
        render();
    }

    shutdown();
    return 0;
}
