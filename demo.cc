#include "demo.h"

#include "mesh.h"
#include "shadermanager.h"
#include "uipainter.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>

#include <GL/glew.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#include <random>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <optional>

using namespace std::string_literals;

namespace GlobalGameParams
{
int ShapeCount = 6; // Default value
// These will store the dimensions passed from JS or defaults
int CanvasWidth = 550;
int CanvasHeight = 900;
int Columns = 2;
}

namespace
{

EM_JS(void, jsEventPusher, (const char *eventNameCStr), {
    const eventName = UTF8ToString(eventNameCStr);
    events.push(eventName);
    console.log("C++ (EM_JS): Pushed event '" + eventName + "' to JS queue. Queue size: " + events.length);
});

auto ShapeSegments = 4;
auto Scale = 1.0;
auto LevelChange = false;

// constexpr const auto TotalPlayTime = 120.0f; // Seconds

constexpr const auto LevelUpScore = 5;
constexpr const auto LevelDownScore = 3;

constexpr const auto TopMargin = 0;

constexpr const auto BackgroundColor = glm::vec3(21.0f / 255.0f, 21.0f / 255.0f, 21.0f / 255.0f);
constexpr const auto Orange = glm::vec4(200.0f / 255.0f, 100.0f / 255.0f, 0.0f / 255.0f, 1.0);
constexpr const auto Yellow = glm::vec4(250.0f / 255.0f, 250.0f / 255.0f, 0.0f / 255.0f, 1.0);

constexpr float vh = 130.0f / 255.0f; // 0.8f
constexpr float vm = 65.0f / 255.0f;  // 0.4f
constexpr float vl = 0.0f / 255.0f;   // 0.0f
constexpr float ga = 0.8f;

constexpr const std::array<glm::vec4, 12> GeneratedColors = {
    // Primary colors (one component 204, others 0)
    glm::vec4(vh, vl, vl, ga), // 1. Red
    glm::vec4(vl, vh, vl, ga), // 2. Green
    glm::vec4(vl, vl, vh, ga), // 3. Blue

    // Secondary colors (two components 204, one 0)
    glm::vec4(vh, vh, vl, ga), // 4. Yellow (204, 204, 0)
    glm::vec4(vh, vl, vh, ga), // 5. Magenta (204, 0, 204)
    glm::vec4(vl, vh, vh, ga), // 6. Cyan (0, 204, 204)

    // Tertiary-like colors (one 204, one 102, one 0)
    glm::vec4(vh, vm, vl, ga), // 7. Orange (204, 102, 0)
    glm::vec4(vh, vl, vm, ga), // 8. Rose (204, 0, 102)
    glm::vec4(vm, vh, vl, ga), // 9. Chartreuse Green (102, 204, 0)
    glm::vec4(vl, vh, vm, ga), // 10. Spring Green (0, 204, 102)
    glm::vec4(vm, vl, vh, ga), // 11. Violet (102, 0, 204)
    glm::vec4(vl, vm, vh, ga)  // 12. Azure (0, 102, 204)
};

constexpr const auto GeneratedGreen = GeneratedColors[1];
constexpr const auto GeneratedCyan = GeneratedColors[5];

constexpr const auto FadeOutTime = 1.0f;
constexpr const auto SuccessStateTime = 1.0f;
constexpr const auto FailStateTime = 0.5f;

constexpr const std::array<glm::imat4x4, 24> Rotations = {
    glm::imat4x4{{0, 0, -1, 0}, {0, -1, 0, 0}, {-1, 0, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 0, -1, 0}, {0, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 0, 1, 0}, {0, -1, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 0, 1, 0}, {0, 1, 0, 0}, {-1, 0, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 0, -1, 0}, {-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 0, -1, 0}, {1, 0, 0, 0}, {0, -1, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 0, 1, 0}, {-1, 0, 0, 0}, {0, -1, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 0, 1, 0}, {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, -1, 0, 0}, {0, 0, -1, 0}, {1, 0, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, -1, 0, 0}, {0, 0, 1, 0}, {-1, 0, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 1, 0, 0}, {0, 0, -1, 0}, {-1, 0, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 1, 0, 0}, {0, 0, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, -1, 0, 0}, {-1, 0, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, -1, 0, 0}, {1, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 1, 0, 0}, {-1, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{0, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{-1, 0, 0, 0}, {0, 0, -1, 0}, {0, -1, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{-1, 0, 0, 0}, {0, 0, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{1, 0, 0, 0}, {0, 0, -1, 0}, {0, 1, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{1, 0, 0, 0}, {0, 0, 1, 0}, {0, -1, 0, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{-1, 0, 0, 0}, {0, -1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{1, 0, 0, 0}, {0, -1, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, 1}},
    glm::imat4x4{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};

constexpr const char *FontName = "monoid-regular.ttf";

float totalPlayTime()
{
    // 60s on iPhone (or default), otherwise 120s on wide/hard
    return GlobalGameParams::ShapeCount * 10.0f;
}

Blocks rotated(const Blocks &shape, const glm::imat4x4 &rotation)
{
    Blocks rotated(shape.size());
    std::transform(shape.begin(), shape.end(), rotated.begin(),
                   [&rotation](const glm::ivec3 &p) { return rotation * glm::ivec4(p, 1); });
    return rotated;
}

Blocks canonicalized(const Blocks &shape)
{
    const auto origin = std::accumulate(shape.begin(), shape.end(), glm::ivec3(std::numeric_limits<int>::max()),
                                        [](const glm::ivec3 &a, const glm::ivec3 &b) { return glm::min(a, b); });
    Blocks result(shape.size());
    std::transform(shape.begin(), shape.end(), result.begin(), [&origin](const glm::ivec3 &p) { return p - origin; });
    std::sort(result.begin(), result.end(), [](const glm::ivec3 &lhs, const glm::ivec3 &rhs) {
        return std::tie(lhs.x, lhs.y, lhs.z) < std::tie(rhs.x, rhs.y, rhs.z);
    });
    return result;
}

bool sameShape(const Blocks &lhs, const Blocks &rhs)
{
    if (lhs.size() != rhs.size())
        return false;
    const Blocks base = canonicalized(lhs);
    for (const auto &rotation : Rotations)
    {
        if (base == canonicalized(rotated(rhs, rotation)))
            return true;
    }
    return false;
}

std::optional<Blocks> generateShape(std::mt19937 &generator)
{
    auto randomBit = [&generator] { return std::uniform_int_distribution<int>(0, 1)(generator); };

    auto center = glm::ivec3(0);
    unsigned direction = 2;
    int side = 1;

    Blocks blocks;
    for (size_t i = 0; i < ShapeSegments; ++i)
    {
        const auto d = 2 * side * glm::ivec3(direction >> 2, (direction >> 1) & 1, direction & 1);
        const auto l = 2 + (i & 1) + randomBit();
        for (int i = 0; i < l; ++i)
        {
            // reject self-intersecting shapes
            if (auto it = std::find(blocks.begin(), blocks.end(), center); it != blocks.end())
                return {};
            blocks.push_back(center);
            center += d;
        }
        switch (direction)
        {
        case 1:
            // 001 -> 010 100
            direction = randomBit() ? 2 : 4;
            break;
        case 2:
            // 010 -> 001 100
            direction = randomBit() ? 1 : 4;
            break;
        case 4:
            // 100 -> 001 010
            direction = randomBit() ? 1 : 2;
            break;
        default:
            assert(false);
            break;
        }
        if (randomBit())
            side = -side;
    }
    return blocks;
}

std::unique_ptr<Shape> initializeShape(const Blocks &blocks, const glm::imat4x4 &baseRotation)
{
    auto makeMesh = [&blocks](float blockScale) {
        struct Vertex
        {
            glm::vec3 position;
            glm::vec2 texCoord;
        };
        std::vector<Vertex> vertices;
        for (const auto &center : blocks)
        {
            const auto p = 1.0f;
            auto addFace = [&vertices, blockScale, center = glm::vec3(center),
                            p](const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3) {
                vertices.push_back({p0 * blockScale + center, {0, 0}});
                vertices.push_back({p1 * blockScale + center, {0, p}});
                vertices.push_back({p2 * blockScale + center, {p, p}});

                vertices.push_back({p2 * blockScale + center, {p, p}});
                vertices.push_back({p3 * blockScale + center, {p, 0}});
                vertices.push_back({p0 * blockScale + center, {0, 0}});
            };

            // top
            addFace(glm::vec3(-p, p, -p), glm::vec3(-p, p, p), glm::vec3(p, p, p), glm::vec3(p, p, -p));
            // bottom
            addFace(glm::vec3(p, -p, -p), glm::vec3(p, -p, p), glm::vec3(-p, -p, p), glm::vec3(-p, -p, -p));
            // right
            addFace(glm::vec3(-p, -p, -p), glm::vec3(-p, -p, p), glm::vec3(-p, p, p), glm::vec3(-p, p, -p));
            // left
            addFace(glm::vec3(p, p, -p), glm::vec3(p, p, p), glm::vec3(p, -p, p), glm::vec3(p, -p, -p));
            // back
            addFace(glm::vec3(-p, -p, -p), glm::vec3(-p, p, -p), glm::vec3(p, p, -p), glm::vec3(p, -p, -p));
            // front
            addFace(glm::vec3(p, -p, p), glm::vec3(p, p, p), glm::vec3(-p, p, p), glm::vec3(-p, -p, p));
        }

        auto mesh = std::make_unique<Mesh>();
        mesh->setVertexCount(vertices.size());
        mesh->setVertexSize(sizeof(Vertex));
        mesh->addVertexAttribute(3, GL_FLOAT, offsetof(Vertex, position));
        mesh->addVertexAttribute(2, GL_FLOAT, offsetof(Vertex, texCoord));
        mesh->initialize();
        mesh->setVertexData(vertices.data());

        return mesh;
    };

    glm::vec3 center = std::accumulate(blocks.begin(), blocks.end(), glm::ivec3(0));
    const auto scaleRatio = 1.0f;
    if (!blocks.empty())
    { // Added check to prevent division by zero, common practice.
        center *= scaleRatio * 1.0f / blocks.size();
    }
    else
    {
        center = glm::vec3(0.0f); // Or handle as an error/specific case
    }

    const auto rx = glm::rotate(glm::mat4(1.0f), 0.25f * glm::pi<float>(), glm::vec3(scaleRatio, 0.0f, 0.0f));
    const auto rz = glm::rotate(glm::mat4(1.0f), 0.25f * glm::pi<float>(), glm::vec3(0.0f, 0.0f, scaleRatio));
    const auto rotation = glm::quat_cast(rx * rz * glm::mat4(baseRotation));

    auto shape = std::make_unique<Shape>();
    shape->blocks = blocks;
    shape->center = center;
    shape->mesh = makeMesh(scaleRatio);
    shape->outlineMesh = makeMesh(1.25f * scaleRatio);
    shape->baseRotation = baseRotation;
    shape->rotation = rotation;

    // --- Add random color selection ---
    static std::mt19937 generator = [] {
        std::random_device rd;
        return std::mt19937(rd());
    }();
    static std::uniform_int_distribution<int> distribution(0, static_cast<int>(GeneratedColors.size() - 1));
    shape->baseColor = GeneratedColors[distribution(generator)];
    // --- End random color selection ---

    return shape;
}

} // End namespace

Demo::Demo(int canvasWidth, int canvasHeight)
    : m_canvasWidth(canvasWidth)
    , m_canvasHeight(canvasHeight)
    , m_shaderManager(new ShaderManager)
    , m_uiPainter(new UIPainter(m_shaderManager.get()))
    , m_shakes(GlobalGameParams::ShapeCount)
{
    m_uiPainter->resize(canvasWidth, canvasHeight);
    initialize();
}

Demo::~Demo() = default;

void Demo::renderAndStep(float elapsed)
{
    render();
    update(elapsed);
}

void Demo::render() const
{
    glClearColor(BackgroundColor.r, BackgroundColor.g, BackgroundColor.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_state != State::Intro && m_state != State::Pause)
        renderShapes();

    renderUI();
}

void Demo::renderShapes() const
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);

    m_shaderManager->useProgram(ShaderManager::Shape);

    const auto viewportWidth = m_canvasWidth / GlobalGameParams::Columns;
    const auto viewportHeight =
        (m_canvasHeight - TopMargin) / ((m_shapes.size() + GlobalGameParams::Columns - 1) / GlobalGameParams::Columns);

    for (size_t i = 0; i < m_shapes.size(); ++i)
    {
        const auto &shape = m_shapes[i];

        const auto viewportX = (i % GlobalGameParams::Columns) * viewportWidth;
        const auto viewportY = (i / GlobalGameParams::Columns) * viewportHeight;
        glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

        const auto projection =
            glm::perspective(glm::radians(45.0f), static_cast<float>(viewportWidth) / viewportHeight, 0.1f, 100.f);

        const auto viewPos = glm::vec3(0, 0, -25);
        const auto viewUp = glm::vec3(0, 1, 0);
        const auto viewCenter = [this, i, &shape] {
            if (m_state == State::Fail && shape->selected)
            {
                const auto &shake = m_shakes[i];
                return glm::vec3(shake.eval(m_stateTime / FailStateTime), 0);
            }
            return glm::vec3(0, 0, 0);
        }();
        const auto view = glm::lookAt(viewPos, viewCenter, viewUp);

        const auto t = glm::translate(glm::mat4(1.0f), -shape->center);

        const auto rotation = [this, i, &shape] {
            if ((m_state == State::Success || m_state == State::Result) && i == m_secondShape)
            {
                const auto targetRotation = m_shapes[m_firstShape]->rotation;
                const auto l = m_state == State::Result ? FadeOutTime : 0.5f * SuccessStateTime;
                const auto t = std::min(1.0f, m_stateTime / l);
                return glm::mix(shape->rotation, targetRotation, t);
            }
            return shape->rotation;
        }();

        auto r = glm::mat4_cast(rotation) * shape->wobble.rotation();
        if (m_state == State::Success || m_state == State::Result)
        {
            if (shape->selected)
            {
                r = glm::mat4_cast(rotation);
            }
        }
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(Scale)); // Last factor for scale
        const auto model = scaleMatrix * r * t;

        const auto mvp = projection * view * model;

        m_shaderManager->setUniform(ShaderManager::ModelViewProjection, mvp);

        if (shape->selected)
        {
            auto color = [this] {
                switch (m_state)
                {
                case State::Fail:
                    return glm::vec3(220.0f / 255.0f, 50.0f / 255.0f, 47.0f / 255.0f);
                case State::Success:
                    return glm::vec3(133.0f / 255.0f, 153.0f / 255.0f, 0.0f / 255.0f);
                default:
                    return glm::vec3(181.0f / 255.0f, 137.0f / 255.0f, 0.0f / 255.0f);
                }
            }();
            if (m_state == State::Result)
                color = glm::mix(color, BackgroundColor, std::min(1.0f, m_stateTime / FadeOutTime));
            m_shaderManager->setUniform(ShaderManager::MixColor, glm::vec4(color, 1.0)); // AAAA
            glDisable(GL_DEPTH_TEST);
            shape->outlineMesh->render(GL_TRIANGLES);
        }

        const auto bgAlpha = [this, i] {
            switch (m_state)
            {
            case State::Result: {
                auto alpha = std::max(.7f, m_stateTime / FadeOutTime);
                if (i == m_firstShape || i == m_secondShape)
                {
                    alpha *= 0.5f;
                    m_shapes[i]->selected = true;
                } else {
                   m_shapes[i]->selected = false; // Unselect wrong matches 
                }

                return alpha;
            }
            case State::Success: {
                if (i != m_firstShape && i != m_secondShape)
                    return std::min(1.0f, m_stateTime / (0.5f * SuccessStateTime));
                break;
            }
            default:
                break;
            }
            return .7f;
        }();
        // m_shaderManager->setUniform(ShaderManager::MixColor, glm::vec4(BackgroundColor, bgAlpha));
        // old cyan: glm::vec4(glm::vec3(42.0f / 255.0f, 161.0f / 255.0f, 152.0f / 255.0f), bgAlpha)
        m_shaderManager->setUniform(ShaderManager::MixColor, shape->baseColor);
        glEnable(GL_DEPTH_TEST);
        shape->mesh->render(GL_TRIANGLES);
    }
}

void Demo::renderUI() const
{
    glViewport(0, 0, m_canvasWidth, m_canvasHeight);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_uiPainter->startPainting();

    switch (m_state)
    {
    case State::Intro:
        renderIntro();
        break;
    case State::Success:
    case State::Fail:
    case State::Playing:
        renderTimer();
        break;
    case State::Pause:
        renderTimer();
        break;
    case State::Result:
        renderTimer();
        renderScore();
        break;
    default:
        break;
    }

    m_uiPainter->donePainting();
}

void Demo::renderTimer() const
{
    static const UIPainter::Font FontBig{FontName, 80};
    static const UIPainter::Font FontSmall{FontName, 40};

    const auto remaining = std::max(0, static_cast<int>((totalPlayTime() - m_playTime) * 1000));
    const auto localState = m_state;
    const auto bigText = [remaining, localState] {
        std::stringstream ss;
        ss.fill('0');
        ss.width(2);
        ss << remaining / 1000 / 60;
        ss << ':';
        ss.fill('0');
        ss.width(2);
        ss << (remaining / 1000) % 60;
        if (localState == State::Pause)
        {
            ss << " (paused)";
        }
        return ss.str();
    }();
    const auto smallText = [remaining] {
        std::stringstream ss;
        ss << '.';
        ss.fill('0');
        ss.width(3);
        ss << remaining % 1000;
        return ss.str();
    }();

    const auto alpha = [this] {
        if (m_state == State::Result)
            return std::max(0.0f, 1.0f - m_stateTime / FadeOutTime);
        return 1.0f;
    }();

    m_uiPainter->setFont(FontBig);
    const auto bigAdvance = m_uiPainter->horizontalAdvance(smallText);

    m_uiPainter->setFont(FontSmall);
    const auto smallAdvance = m_uiPainter->horizontalAdvance(smallText);

    const auto totalAdvance = bigAdvance; // + smallAdvance;
    // Note that the coordinates have 0:0 in the center
    // and there is something funky about the assumed size
    // of the SDL area I think, otherwise I would not have
    // all these weird things.
    const auto top = m_canvasHeight > 600 ? -0.78 * m_canvasHeight : -0.45 * m_canvasHeight;

    const auto textPos = glm::vec2(-0.5 * totalAdvance, top);

    m_uiPainter->setFont(FontSmall);
    m_uiPainter->drawText(textPos, glm::vec4(42.0f / 255.0f, 161.0f / 255.0f, 152.0f / 255.0f, alpha), 0, bigText);

    const auto right = m_canvasWidth > 600 ? 0.35 * m_canvasWidth : 0.5 * m_canvasWidth;
    const auto scorePos = glm::vec2(right, top);

    const auto local_score = m_score;

    const auto score = [local_score] {
        std::stringstream ss;
        ss << local_score;
        return ss.str();
    }();

    m_uiPainter->setFont(FontSmall);
    m_uiPainter->drawText(scorePos, Orange, 0, score);

    // m_uiPainter->setFont(FontSmall);
    // m_uiPainter->drawText(textPos + glm::vec2(bigAdvance, 0), glm::vec4(42.0f/255.0f, 161.0f/255.0f, 152.0f/255.0f,
    // alpha), 0, smallText);
}

void Demo::renderIntro() const
{
    static const UIPainter::Font FontHuge{FontName, 80};
    static const UIPainter::Font FontBig{FontName, 60};
    static const UIPainter::Font FontSmall{FontName, 40};

    const auto color = glm::vec4(42.0f / 255.0f, 161.0f / 255.0f, 152.0f / 255.0f, 1.0);

    m_uiPainter->setFont(FontHuge);
    drawCenteredText(glm::vec2(0, -120), GeneratedCyan, "ROTATOR"s);
    m_uiPainter->setFont(FontBig);
    drawCenteredText(glm::vec2(0, -40), color, "SELECT THE MATCHING"s);
    drawCenteredText(glm::vec2(0, 10), color, "PAIR OF SHAPES"s);

    m_uiPainter->setFont(FontSmall);
    drawCenteredText(glm::vec2(0, 200), GeneratedGreen, "TAP TO START"s);
}

void Demo::renderScore() const
{
    static const UIPainter::Font FontBig{FontName, 60};
    static const UIPainter::Font FontSmall{FontName, 40};

    const auto alpha = [this] {
        constexpr auto StartTime = 2.0f;
        constexpr auto FadeInTime = 1.0f;
        if (m_stateTime < StartTime)
            return 0.0f;
        return std::min(1.0f, (m_stateTime - StartTime) / FadeInTime);
    }();
    const auto color = Yellow;

    m_uiPainter->setFont(FontBig);

    const auto scoreText = [this] {
        std::stringstream ss;
        ss << m_score << " SHAPES ROTATED";
        return ss.str();
    }();
    drawCenteredText(glm::vec2(0, -40), color, scoreText);

    if (m_score > 0)
    {
        /*const auto accuracyText = [this] {
            std::stringstream ss;
            ss << "ACCURACY: ";
            ss << std::fixed;
            ss << std::setprecision(2);
            ss << m_score * 100.0f / m_attempts;
            ss << "%";
            return ss.str();
        }();*/
        if (m_score >= LevelUpScore && LevelChange)
        {
            drawCenteredText(glm::vec2(0, 20), color, "LEVEL UP"s);
        }
    }
    if (m_score <= LevelDownScore && LevelChange)
    {
        drawCenteredText(glm::vec2(0, 20), color, "LEVEL DOWN"s);
    }

    m_uiPainter->setFont(FontSmall);
    drawCenteredText(glm::vec2(0, 200), color, "TAP TO CONTINUE"s);
}

void Demo::drawCenteredText(const glm::vec2 &pos, const glm::vec4 &color, const std::string &text) const
{
    const auto advance = m_uiPainter->horizontalAdvance(text);
    m_uiPainter->drawText(pos - glm::vec2(0.5f * advance, 0.0f), color, 0, text);
}

void Demo::update(float elapsed)
{
    if (m_state == State::Pause)
    {
        return;
    }
    if (m_state != State::Success)
    {
        for (auto &shape : m_shapes)
        {
            shape->wobble.update(elapsed);
        }
    }
    m_stateTime += elapsed;
    switch (m_state)
    {
    case State::Pause:
        break;
    case State::Intro:
        break;
    case State::Success:
        if (m_stateTime > SuccessStateTime)
        {
            setState(State::Playing);
            initializeShapes();
        }
        break;
    case State::Fail:
        if (m_stateTime > FailStateTime)
        {
            for (auto &shape : m_shapes)
                shape->selected = false;
            m_selectedCount = 0;
            setState(State::Playing);
        }
        [[fallthrough]];
    case State::Playing:
        m_playTime += elapsed;
        if (m_playTime > totalPlayTime())
        {
            if (m_state != State::Result)
            {
                LevelChange = false;
                if (m_score >= LevelUpScore)
                {
                    if (ShapeSegments <= 6)
                    {
                        ShapeSegments++;
                        LevelChange = true;
                    }
                }
                if (m_score <= LevelDownScore)
                {
                    if (ShapeSegments >= 5)
                    {
                        ShapeSegments--;
                        LevelChange = true;
                    }
                }
                switch (ShapeSegments)
                {
                case 4:
                    Scale = 1.0;
                    break;
                case 5:
                    Scale = 0.9;
                    break;
                case 6:
                    Scale = 0.8;
                    break;
                case 7:
                    Scale = 0.7;
                    break;
                }
            }
            setState(State::Result);
        }

        break;
    case State::Result:
        break;
    }
}

void Demo::initialize()
{
    m_score = 0;
    m_attempts = 0;
    m_playTime = 0.0f;
    initializeShapes();
}

void Demo::initializeShapes()
{
    static auto generator = [] {
        std::random_device rd;
        return std::mt19937(rd());
    }();
    m_firstShape = std::uniform_int_distribution<int>(0, GlobalGameParams::ShapeCount - 2)(generator);
    m_secondShape = std::uniform_int_distribution<int>(m_firstShape + 1, GlobalGameParams::ShapeCount - 1)(generator);

    m_shapes.clear();
    for (int i = 0; i < GlobalGameParams::ShapeCount; ++i)
    {
        auto blocks = [this, i] {
            if (i == m_secondShape)
                return m_shapes[m_firstShape]->blocks;
            std::uniform_int_distribution<size_t> distribution(0, (1 << (3 * ShapeSegments)) - 1);
            std::optional<Blocks> blocks;
            for (;;)
            {
                blocks = generateShape(generator);
                const auto valid = [this, i, &blocks] {
                    if (!blocks)
                        return false;
                    auto it = std::find_if(m_shapes.begin(), m_shapes.end(),
                                           [&blocks](const auto &shape) { return sameShape(*blocks, shape->blocks); });
                    return it == m_shapes.end();
                }();
                if (valid)
                    break;
            }
            return *blocks;
        }();
        auto rotation = [this, i, &blocks] {
            glm::imat4x4 rotation;
            for (;;)
            {
                const auto index = std::uniform_int_distribution<int>(0, Rotations.size() - 1)(generator);
                rotation = Rotations[index];
                const auto valid = [this, i, &blocks, &rotation] {
                    if (i != m_secondShape)
                        return true;
                    const auto &firstShape = m_shapes[m_firstShape];
                    const Blocks base = canonicalized(rotated(firstShape->blocks, firstShape->baseRotation));
                    return base != canonicalized(rotated(blocks, rotation));
                }();
                if (valid)
                    break;
            }
            return rotation;
        }();
        m_shapes.push_back(initializeShape(blocks, rotation));
    }

    assert(sameShape(m_shapes[m_firstShape]->blocks, m_shapes[m_secondShape]->blocks));

    m_selectedCount = 0;
}

void Demo::handleKeyPress(Key)
{
    switch (m_state)
    {
    case State::Intro:
        setState(State::Playing);
        break;
#ifdef CHEAT
    case State::Playing: {
        if (!m_shapes[m_firstShape]->selected)
            toggleShapeSelection(m_firstShape);
        else
            toggleShapeSelection(m_secondShape);
        break;
    }
#endif
    case State::Result: {
        if (m_stateTime > FadeOutTime)
        {
            setState(State::Playing);
            initialize();
        }
        break;
    }
    default:
        break;
    }
}

void Demo::handleMouseButton(int x, int y)
{
    switch (m_state)
    {
    case State::Intro:
        setState(State::Playing);
        break;
    case State::Playing: {
        if (y < 30)
        {
            setState(State::Pause);
            glClearColor(BackgroundColor.r, BackgroundColor.g, BackgroundColor.b, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            break;
        }
        const auto shapeIndex = [this, x, y = m_canvasHeight - y] {
            const auto viewportWidth = m_canvasWidth / GlobalGameParams::Columns;
            const auto viewportHeight = (m_canvasHeight - TopMargin) /
                                        ((m_shapes.size() + GlobalGameParams::Columns - 1) / GlobalGameParams::Columns);
            const auto row = y / viewportHeight;
            const auto col = x / viewportWidth;
            return row * GlobalGameParams::Columns + col;
        }();
        toggleShapeSelection(shapeIndex);
        break;
    }
    case State::Result: {
        if (m_stateTime > FadeOutTime)
        {
            setState(State::Playing);
            initialize();
        }
        break;
    }
    case State::Pause: {
        setState(State::Playing); // This could put the system in a wrong state in an edge case (when pausing and not
                                  // playing)
    }
    default:
        break;
    }
}

void Demo::setState(State state)
{
    m_state = state;
    m_stateTime = 0.0f;
}

void Demo::toggleShapeSelection(int index)
{
    if (index >= m_shapes.size())
        return;
    auto &shape = m_shapes[index];
    jsEventPusher("select");
    if (shape->selected)
    {
        shape->selected = false;
        --m_selectedCount;
    }
    else
    {
        shape->selected = true;
        ++m_selectedCount;
        if (m_selectedCount == 2)
        {
            ++m_attempts;
            if (m_shapes[m_firstShape]->selected && m_shapes[m_secondShape]->selected)
            {
                ++m_score;
                setState(State::Success);
                //jsEventPusher("success");
            }
            else
            {
                setState(State::Fail);
                //jsEventPusher("fail");
            }
        }
    }
}

extern "C" {
EMSCRIPTEN_KEEPALIVE
void set_shape_count(int count)
{
    GlobalGameParams::ShapeCount = count;
}

void set_columns(int num)
{
    GlobalGameParams::Columns = num;
}

EMSCRIPTEN_KEEPALIVE
void set_canvas_dimensions(int width, int height)
{
    GlobalGameParams::CanvasWidth = width;
    GlobalGameParams::CanvasHeight = height;
    // No direct call to emscripten_set_canvas_element_size here for initial setup.
    // If dynamic resize *after* SDL_Init is needed, then it might be called from JS directly
    // or from C++ if properly linked.
}
} // extern "C"