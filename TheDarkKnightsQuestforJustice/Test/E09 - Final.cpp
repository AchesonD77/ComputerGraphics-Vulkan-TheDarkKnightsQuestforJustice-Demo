#include "modules/Starter.hpp"
#include "modules/TextMaker.hpp"


std::vector<SingleText> outText = {
    {4, {"Ciao, Dark Knight, Using H to turn on/off your flashlight, and 1,2,3,4,5 can change colors of the flashlight",
        "Go to help the Batman finding the Joker, push 9 you can tranfer to near Batman",
        "The Dark Knight's Quest for Justice: When you see the joker, you can take it and push 0 to transfer to the space staion, then go back to your hometown",
        "Good luck, Dark Knight! Enjor your journey of discovery! ",}, 0, 0},
};

// The uniform buffer object used in this example
struct BlinnUniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
    alignas(16) glm::mat4 mMat;
    alignas(16) glm::mat4 nMat;
};

// for sun
struct EmitUniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
};

struct GlobalUniformBufferObject {
    alignas(16) glm::vec3 lightDir;
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec3 eyePos;
    alignas(16) glm::vec3 spotlightPos; // flshlight position
    alignas(16) glm::vec3 spotlightDir; // f direction
    alignas(16) float spotlightCutoff;  // f angles
};
// for cartoon shadering
struct ToonUniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
    alignas(16) glm::mat4 mMat;
    alignas(16) glm::mat4 nMat;
};

// for skybox
struct skyBoxUniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
};

// The vertices data structures
struct BlinnVertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 UV;
};

struct ToonVertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 UV;
};

struct EmitVertex {
    glm::vec3 pos;
    glm::vec2 UV;
};

struct skyBoxVertex {
    glm::vec3 pos;
};


// MAIN !
class E09 : public BaseProject {
    protected:

    //Global Variables and Data Structures: Defines the data needed for rendering
    //(e.g., uniform buffers, vertex descriptors, pipelines)

    // define global variable
    int currentLightColor = 0; // indux for changing the colors
    // for moving to one new point
    bool moveToPosition = false;
    bool debounceKey0 = false;
    int debounceCounterKey0 = 0;
    bool moveToPosition9 = false;
    bool debounceKey9 = false;
    int debounceCounterKey9 = 0;
    
    // define descriptor set layout
    DescriptorSetLayout DSLGlobal;    // For Global values
    DescriptorSetLayout DSLBlinn;    // For Blinn Objects
    DescriptorSetLayout DSLEmit;    // DSL for emission
    DescriptorSetLayout DSLskyBox;    // For skyBox
    DescriptorSetLayout DSLToon; // for toon obj

    // define vertex descriptor
    VertexDescriptor VDBlinn;
    VertexDescriptor VDEmit;
    VertexDescriptor VDskyBox;
    VertexDescriptor VDToon;

    // define pipeline
    Pipeline PBlinn;
    Pipeline PEmit;
    Pipeline PskyBox;
    Pipeline PToon;
    
    // define textmaker
    TextMaker txt;

    // define Models, textures and Descriptor Sets (values assigned to the uniforms) for obj
    Model Mship;
    Texture Tship;
    Model Msun;
    Texture Tsun, Tstars;
    Model M0, M1, M2, M3, M4, M5, M6, M7, M8, M90, M10;
    Model M20, M21;
    Texture T0, T1, T2, T3, T4, T5 , T6, T7, T8, T901, T902, T903, T10;
    Texture T20, T21;
    // skybox
    Model MskyBox;
    Texture TskyBox;

    // define descriptor sets for obj
    DescriptorSet DSship, DSGlobal, DSskyBox;
    DescriptorSet DSsun, DSM0, DSM1, DSM2, DSM3, DSM4, DSM5, DSM6, DSM7, DSM8, DSM90, DSM10;
    DescriptorSet DSM20, DSM21;

    // Other application parameters
    int currScene = 0;
    int subpass = 0;
    
    // define the initial camera position
    glm::vec3 CamPos = glm::vec3(8.11401, 4.68533, -49.0116);// x left and right，y up and down，z front back
    float CamAlpha = glm::radians(-135.0f); // rotate around y-axis  （-45 means turn left)
    float CamBeta = glm::radians(-3.0f); // -10 means go down 10°
    float Ar;
    
    // Here you set the main application parameters
    // set our window size
    void setWindowParameters() {
        // window size, titile and initial background
        windowWidth = 1200;
        windowHeight = 900;
        windowTitle = "The Dark Knight's Quest for Justice";
        windowResizable = GLFW_TRUE;
        initialBackgroundColor = {0.1f, 0.1f, 0.1f, 1.0f};
        Ar = (float)windowWidth / (float)windowHeight;
    }
    
    // What to do when the window changes size
    void onWindowResize(int w, int h) {
        std::cout << "Window resized to: " << w << " x " << h << "\n";
        Ar = (float)w / (float)h;
    }
    
    // Here you load and setup all your Vulkan Models and Texutures.
    // Here you also create your Descriptor set layouts and load the shaders for the pipelines
    void localInit() {
        // Descriptor Layouts [what will be passed to the shaders]
        // Descriptor layouts define the types of data passed to shaders
        DSLGlobal.init(this, {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(GlobalUniformBufferObject), 1}
        });

        DSLBlinn.init(this, {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(BlinnUniformBufferObject), 1},
                    {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1},
            // for three light reflections
            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1}, // normal texture
            {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1}  // Specular highlight reflection
                });
        
        DSLToon.init(this, {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(ToonUniformBufferObject), 1},
                    {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1},
            // for three light reflections
            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1}, // normal texture
            {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1}  // Specular highlight reflection
                });
        
        DSLEmit.init(this, {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(EmitUniformBufferObject), 1},
                    {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1}
                });
        // skybox
        DSLskyBox.init(this, {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(skyBoxUniformBufferObject), 1},
                    {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1},
                    {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1}
                });

        // Vertex descriptors Specify how vertex data is structured and mapped to shaders
        VDBlinn.init(this, {
                  {0, sizeof(BlinnVertex), VK_VERTEX_INPUT_RATE_VERTEX}
                }, {
                  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(BlinnVertex, pos),
                         sizeof(glm::vec3), POSITION},
                  {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(BlinnVertex, norm),
                         sizeof(glm::vec3), NORMAL},
                  {0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(BlinnVertex, UV),
                         sizeof(glm::vec2), UV}
                });

        VDToon.init(this, {
                  {0, sizeof(ToonVertex), VK_VERTEX_INPUT_RATE_VERTEX}
                }, {
                  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(ToonVertex, pos),
                         sizeof(glm::vec3), POSITION},
                  {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(ToonVertex, norm),
                         sizeof(glm::vec3), NORMAL},
                  {0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(ToonVertex, UV),
                         sizeof(glm::vec2), UV}
                });
        
        VDEmit.init(this, {
                  {0, sizeof(EmitVertex), VK_VERTEX_INPUT_RATE_VERTEX}
                }, {
                  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(EmitVertex, pos),
                         sizeof(glm::vec3), POSITION},
                  {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(EmitVertex, UV),
                         sizeof(glm::vec2), UV}
                });
        // Skybox
        VDskyBox.init(this, {
                  {0, sizeof(skyBoxVertex), VK_VERTEX_INPUT_RATE_VERTEX}
            }, {
              {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(skyBoxVertex, pos),
                     sizeof(glm::vec3), POSITION}
            });
        
        

        // Pipelines [Shader couples] initialize Configures shader programs and rendering states
        PBlinn.init(this, &VDBlinn,  "shaders/ToonVert.spv",    "shaders/ToonFrag.spv", {&DSLGlobal, &DSLBlinn});  // with toon
        PToon.init(this, &VDToon, "shaders/BlinnVert.spv", "shaders/BlinnFrag.spv", {&DSLGlobal, &DSLToon}); //without toon
        PEmit.init(this, &VDEmit,  "shaders/EmitVert.spv",    "shaders/EmitFrag.spv", {&DSLEmit});
        PskyBox.init(this, &VDskyBox, "shaders/SkyBoxVert.spv", "shaders/SkyBoxFrag.spv", { &DSLskyBox });
        PskyBox.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,VK_CULL_MODE_BACK_BIT, false);

        // initialize models
        Mship.init(this, &VDToon, "models/AK47.obj", OBJ);
        Msun.init(this, &VDEmit, "models/Sphere.obj", OBJ);
        MskyBox.init(this, &VDskyBox, "models/SkyBoxUni2.obj", OBJ); //different skybox: Skybox2 SkyBoxCube LargePlane SkyBoxUniversity SkyBoxUni2 EarthO model

        M0.init(this, &VDBlinn, "models/Park.obj", OBJ);  // for carpark
        M1.init(this, &VDBlinn, "models/TeslaCar.obj", OBJ); // tesla
        M2.init(this, &VDBlinn, "models/FordGT.obj", OBJ); // fordGT
        // from M3 to M10
        M3.init(this, &VDBlinn, "models/BenzAMGCLA.obj", OBJ); // Benz AMGCLA
        M4.init(this, &VDBlinn, "models/Lambroghini2023.obj", OBJ); // lambroghini2023
        M5.init(this, &VDBlinn, "models/Ferrari01.obj", OBJ);  // ferrari
        M6.init(this, &VDBlinn, "models/Landrover2023.obj", OBJ); // Landrover2023
        M7.init(this, &VDBlinn, "models/Lambroghini2019.obj", OBJ); // Lambroghini2019
        M8.init(this, &VDBlinn, "models/Maserati2024.obj", OBJ); // Maserati2024
        M90.init(this, &VDToon, "models/Satellite1.obj", OBJ); // Satellite1
        M10.init(this, &VDBlinn, "models/Batman.obj", OBJ); // batman
        //from M20
        M20.init(this, &VDToon, "models/Joker2.obj", OBJ); // joker
        M21.init(this, &VDToon, "models/Spaceshuttle.obj", OBJ); // spaceshuttle NASA
        
        // initialize the textures
        Tship.init(this, "textures/AK472.jpeg"); //ak47
        Tsun.init(this, "textures/2k_sun.jpg"); //2k_sun.jpg
        TskyBox.init(this, "textures/galaxystars.jpg");  // name for other skybox: NightCity worlde Space EarthO ground
        Tstars.init(this, "textures/constellation_figures.png");
        T0.init(this, "textures/ground.jpg"); // textures0 for park
        T1.init(this, "textures/Tesla.jpeg"); // tesla
        T2.init(this, "textures/FordGTy.jpg"); // ford GT
        T3.init(this, "textures/BenzAMGCLA.jpg"); // BenzAMGCLA
        T4.init(this, "textures/Lambroghini2023.png"); // Lambroghini2023
        T5.init(this, "textures/Ferrari01.jpeg"); // Ferrari01
        T6.init(this, "textures/Landrover2023.jpeg"); // Landrover2023
        T7.init(this, "textures/Lambroghini2019.jpg"); // Lambroghini2019
        T8.init(this, "textures/Maserati2024.jpeg"); //Maserati2024
        T901.init(this, "textures/CityPark.jpeg"); // for satellite
        T902.init(this, "textures/Hands2.jpeg");
        T903.init(this, "textures/Hands1.jpeg");
        T10.init(this, "textures/BatmanA.jpg"); // batman
        // from t20
        T20.init(this, "textures/Joker2.jpeg"); // for joker
        T21.init(this, "textures/Spaceshuttle.jpeg"); // for spaceshuttle
        
        
        DPSZs.uniformBlocksInPool = 50;
        DPSZs.texturesInPool = 50;
        DPSZs.setsInPool = 50;

        std::cout << "Initializing text\n";
        txt.init(this, &outText);

        std::cout << "Initialization completed!\n";
        std::cout << "Uniform Blocks in the Pool  : " << DPSZs.uniformBlocksInPool << "\n";
        std::cout << "Textures in the Pool        : " << DPSZs.texturesInPool << "\n";
        std::cout << "Descriptor Sets in the Pool : " << DPSZs.setsInPool << "\n";
    }
    
    // Here you create your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsInit() {
        // This creates a new pipeline (with the current surface), using its shaders
        PBlinn.create();
        PEmit.create();
        PskyBox.create();
        PToon.create();
        
        // Here you define the data set
        DSship.init(this, &DSLToon, {&Tship});
        DSGlobal.init(this, &DSLGlobal, {});
        DSskyBox.init(this, &DSLskyBox, { &TskyBox, &Tstars });
        DSsun.init(this, &DSLEmit, {&Tsun});
        
        DSM0.init(this, &DSLBlinn, {&T0});
        DSM1.init(this, &DSLBlinn, {&T1});
        DSM2.init(this, &DSLBlinn, {&T2});
        DSM3.init(this, &DSLBlinn, {&T3});
        DSM4.init(this, &DSLBlinn, {&T4});
        DSM5.init(this, &DSLBlinn, {&T5});
        DSM6.init(this, &DSLBlinn, {&T6});
        DSM7.init(this, &DSLBlinn, {&T7});
        DSM8.init(this, &DSLBlinn, {&T8});
        DSM90.init(this, &DSLToon, {&T901, &T902, &T903});
        DSM10.init(this, &DSLBlinn, {&T10});
        //20
        DSM20.init(this, &DSLToon, {&T20});
        DSM21.init(this, &DSLToon, {&T21});

        txt.pipelinesAndDescriptorSetsInit();
    }


    void pipelinesAndDescriptorSetsCleanup() {
        // Cleanup pipelines
        PBlinn.cleanup();
        PEmit.cleanup();
        PskyBox.cleanup();
        PToon.cleanup();
        
        DSship.cleanup();
        DSsun.cleanup();
        DSskyBox.cleanup();
        DSGlobal.cleanup();
        DSM0.cleanup();
        DSM1.cleanup();
        DSM2.cleanup();
        DSM3.cleanup();
        DSM4.cleanup();
        DSM5.cleanup();
        DSM6.cleanup();
        DSM7.cleanup();
        DSM8.cleanup();
        DSM90.cleanup();
        DSM10.cleanup();
        DSM20.cleanup();
        DSM21.cleanup();
            
        txt.pipelinesAndDescriptorSetsCleanup();
    }


    void localCleanup() {
        Tship.cleanup();
        Mship.cleanup();
        TskyBox.cleanup();
        T0.cleanup();
        T1.cleanup();
        T2.cleanup();
        Tstars.cleanup();
        Tsun.cleanup();
        Msun.cleanup();
        MskyBox.cleanup();
        M0.cleanup();
        M1.cleanup();
        M2.cleanup();
        M3.cleanup();
        T3.cleanup();
        M4.cleanup();
        T4.cleanup();
        M5.cleanup();
        T5.cleanup();
        M6.cleanup();
        T6.cleanup();
        M7.cleanup();
        T7.cleanup();
        M8.cleanup();
        T8.cleanup();
        M90.cleanup();
        T901.cleanup();
        T902.cleanup();
        T903.cleanup();
        M10.cleanup();
        T10.cleanup();
        T20.cleanup();
        M20.cleanup();
        T21.cleanup();
        M21.cleanup();
        
        // Cleanup descriptor set layouts
        DSLBlinn.cleanup();
        DSLEmit.cleanup();
        DSLskyBox.cleanup();
        DSLGlobal.cleanup();
        DSLToon.cleanup();

        // Destroies the pipelines
        PBlinn.destroy();
        PEmit.destroy();
        PskyBox.destroy();
        PToon.destroy();
        
        txt.localCleanup();
    }
    
    // Rendering Models
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
        
        // render M0 mdel to M10, and M20 to M21
        PBlinn.bind(commandBuffer); // Bind the pipeline
        DSGlobal.bind(commandBuffer, PBlinn, 0, currentImage); // Bind descriptor sets (global and object-specific)
        
        M0.bind(commandBuffer);
        DSM0.bind(commandBuffer, PBlinn, 1, currentImage);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M0.indices.size()), 1, 0, 0, 0); // Issue a draw call with vkCmdDrawIndexed
        
        // render m1 model
        M1.bind(commandBuffer);
        DSM1.bind(commandBuffer, PBlinn, 1, currentImage);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M1.indices.size()), 1, 0, 0, 0);
        
        M2.bind(commandBuffer);
        DSM2.bind(commandBuffer, PBlinn, 1, currentImage);     // Bind descriptor sets of M2
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M2.indices.size()), 1, 0, 0, 0); // drwa M2
        
        M3.bind(commandBuffer);
        DSM3.bind(commandBuffer, PBlinn, 1, currentImage);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M3.indices.size()), 1, 0, 0, 0);
        
        M4.bind(commandBuffer);
        DSM4.bind(commandBuffer, PBlinn, 1, currentImage);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M4.indices.size()), 1, 0, 0, 0);
        
        M5.bind(commandBuffer);
        DSM5.bind(commandBuffer, PBlinn, 1, currentImage);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M5.indices.size()), 1, 0, 0, 0);
        
        M6.bind(commandBuffer);
        DSM6.bind(commandBuffer, PBlinn, 1, currentImage);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M6.indices.size()), 1, 0, 0, 0);
        
        M7.bind(commandBuffer);
        DSM7.bind(commandBuffer, PBlinn, 1, currentImage);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M7.indices.size()), 1, 0, 0, 0);
        
        M8.bind(commandBuffer);
        DSM8.bind(commandBuffer, PBlinn, 1, currentImage);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M8.indices.size()), 1, 0, 0, 0);
        
        // binds the pipeline
        PToon.bind(commandBuffer);
        // The models (both index and vertex buffers)
        M90.bind(commandBuffer);
        DSGlobal.bind(commandBuffer, PToon, 0, currentImage);    // The Global Descriptor Set (Set 0)
        DSM90.bind(commandBuffer, PToon, 1, currentImage);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M90.indices.size()), 1, 0, 0, 0);
        
        M10.bind(commandBuffer);
        DSM10.bind(commandBuffer, PBlinn, 1, currentImage);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M10.indices.size()), 1, 0, 0, 0);
        
        PToon.bind(commandBuffer);
        M20.bind(commandBuffer);
        DSM20.bind(commandBuffer, PToon, 1, currentImage);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M20.indices.size()), 1, 0, 0, 0);

        PToon.bind(commandBuffer);
        M21.bind(commandBuffer);
        DSM21.bind(commandBuffer, PToon, 1, currentImage);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M21.indices.size()), 1, 0, 0, 0);
        
        // binds the pipeline
        PToon.bind(commandBuffer);
        // The models (both index and vertex buffers)
        Mship.bind(commandBuffer);
        // The descriptor sets, for each descriptor set specified in the pipeline
        DSGlobal.bind(commandBuffer, PToon, 0, currentImage);    // The Global Descriptor Set (Set 0)
        DSship.bind(commandBuffer, PToon, 1, currentImage);    // The Material and Position Descriptor Set (Set 1)
        // The actual draw call.
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(Mship.indices.size()), 1, 0, 0, 0);
        
        // Skybox rendering
        PskyBox.bind(commandBuffer);
        MskyBox.bind(commandBuffer);
        DSskyBox.bind(commandBuffer, PskyBox, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MskyBox.indices.size()), 1, 0, 0, 0);
        
        // render sun
        // binds the pipeline
        PEmit.bind(commandBuffer);
        // The models (both index and vertex buffers)
        Msun.bind(commandBuffer);
        // The descriptor sets, for each descriptor set specified in the pipeline
        DSsun.bind(commandBuffer, PEmit, 0, currentImage);
        // The actual draw call.
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(Msun.indices.size()), 1, 0, 0, 0);
        
        // Render text overlay
        std::cout << "Rendering text for current scene." << std::endl;
        txt.populateCommandBuffer(commandBuffer, currentImage, currScene);
        
    }
    
    void updateUniformBuffer(uint32_t currentImage) {
        
        static bool isPerspective = true; // Projection function: Initial perspective projection
        static bool debounce = false;
        static int curDebounce = 0;
        
        // Camera position and Angle, takes input (keyboard or mouse), and converts it to orientation shift m and rotation r
        float deltaT;
        glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
        bool fire = false;
        getSixAxis(deltaT, m, r, fire);
        
        static float autoTime = true;
        static float cTime = 0.0;
        const float turnTime = 36.0f;
        const float angTurnTimeFact = 2.0f * M_PI / turnTime;
        
        if(autoTime) {
            cTime = cTime + deltaT;
            cTime = (cTime > turnTime) ? (cTime - turnTime) : cTime;
        }
        cTime += r.z * angTurnTimeFact * 4.0;
        
        // update the camere's
        const float ROT_SPEED = glm::radians(140.0f); // rotatation speed
        const float MOVE_SPEED = 3.0f;
        
        CamAlpha = CamAlpha - ROT_SPEED * deltaT * r.y; // horizontal rotation
        CamBeta  = CamBeta  - ROT_SPEED * deltaT * r.x; // vertical roration
        CamBeta  =  CamBeta < glm::radians(-90.0f) ? glm::radians(-90.0f) :
        (CamBeta > glm::radians( 90.0f) ? glm::radians( 90.0f) : CamBeta);
        
        glm::vec3 ux = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0,1,0)) * glm::vec4(1,0,0,1); // x direction
        glm::vec3 uz = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0,1,0)) * glm::vec4(0,0,1,1); // z direction
        CamPos = CamPos + MOVE_SPEED * m.x * ux * deltaT; // left and right moving
        CamPos = CamPos + MOVE_SPEED * m.y * glm::vec3(0,1,0) * deltaT; // up and down moving
        CamPos = CamPos + MOVE_SPEED * m.z * uz * deltaT; // front and back moving
        // debug statement
        std::cout << "Camera Position Updated: ("
        << CamPos.x << ", " << CamPos.y << ", " << CamPos.z << ")\n";
        
        // keyboard inputs control various gameplay features:
        
        // Uniform update（camera's MVP matrix）
        glm::mat4 M;
        if (isPerspective) {
            M = glm::perspective(glm::radians(60.0f), Ar, 0.1f, 100.0f); // perspective projection
        } else {
            float orthoScale = 10.0f; // the scale of the orthogonal projection
            M = glm::ortho(-Ar * orthoScale, Ar * orthoScale, -orthoScale, orthoScale, 0.1f, 100.0f); // orthogonal projection
        }
        M[1][1] *= -1; // Vulkan clip space Y axis
        
        // The view matrix is generated through the CamPos of the camera and the rotation angles CamAlpha/CamBeta.
        // Mv four-view matrix, which is used to transform the world space to the camera space
        glm::mat4 Mv = glm::rotate(glm::mat4(1.0), -CamBeta, glm::vec3(1, 0, 0)) *
        glm::rotate(glm::mat4(1.0), -CamAlpha, glm::vec3(0, 1, 0)) *
        glm::translate(glm::mat4(1.0), -CamPos);
        
        glm::mat4 ViewPrj = M * Mv;
        
        // first-person perspetive - AK47
        BlinnUniformBufferObject blinnUboBoat{};
        // FP position
        glm::vec3 forward = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 0);
        glm::vec3 boatPosition = CamPos + forward * 1.5f; // front and back with camera
        glm::mat4 translationBoat = glm::translate(glm::mat4(1.0f), boatPosition - glm::vec3(0, 0.5f, 0)); // towards Y axis down 0.5
        
        // The rotation of the boat is adjusted to be in the same direction as the camera and rotated 90° to the left
        glm::mat4 rotationCamera = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0, 1, 0));
        glm::mat4 rotationBoatLeft = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0)); // front and back with camera
        glm::mat4 rotationBoat = rotationCamera * rotationBoatLeft;
        // adjust sacle for FP perspetive - AK47
        glm::mat4 scaleBoat = glm::scale(glm::mat4(1.0f), glm::vec3(0.006f));
        // Merging transformation matrices
        blinnUboBoat.mMat = translationBoat * rotationBoat * scaleBoat;
        blinnUboBoat.mvpMat = ViewPrj * blinnUboBoat.mMat;
        blinnUboBoat.nMat = glm::inverse(glm::transpose(blinnUboBoat.mMat));
        // Map the updated uniform buffer
        DSship.map(currentImage, &blinnUboBoat, 0);
        
        // updates global uniforms (light and color and position)
        GlobalUniformBufferObject gubo{};
        gubo.lightDir = glm::vec3(0.0f, 1.0f, 0.0f); // The direction of light from top to bottom
        gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // Highlight the white light
        gubo.eyePos = CamPos;
        DSGlobal.map(currentImage, &gubo, 0);
        std::cout << "Current Spotlight Cutoff Angle: " << glm::degrees(acos(gubo.spotlightCutoff)) << "°" << std::endl;
        
        // define different colors
        glm::vec3 lightColorOptions[5] = {
            //            glm::vec3(1.5, 1.2, 1.0), // Warm white light
            //            glm::vec3(1.2, 1.4, 1.5), // Cold white light
            //            glm::vec3(0.8, 0.9, 1.5), // blue light
            //            glm::vec3(0.5, 1.2, 0.5)  // green light
            glm::vec3(1.5, 1.5, 1.5), // standard whit light
            glm::vec3(1.8, 1.5, 1.2), // warm yellow light
            glm::vec3(0.8, 0.9, 1.5), // cold blue light
            glm::vec3(0.5, 1.0, 0.5), // Pale green light
            glm::vec3(1.5, 0.3, 0.3)  // red light
        };
        
        // Detect keys 1-4 to switch the lighting color
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) currentLightColor = 0;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) currentLightColor = 1;
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) currentLightColor = 2;
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) currentLightColor = 3;
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) currentLightColor = 4;
        // push H to turn on/off the light
        static bool flashlightOn = false;  // switch
        static bool debounceH = false; // Stabilization treatment
        static int curDebounceH = 0;
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !debounceH) {
            flashlightOn = !flashlightOn; // switch the light on/off
            debounceH = true;
            curDebounceH = 15; // key cooldown time
        }
        if (curDebounceH > 0) {
            curDebounceH--;
            if (curDebounceH == 0) {
                debounceH = false;
            }
        }
        
        // 0 to change the position
        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS && !debounceKey0) {
            moveToPosition = true;  // true
            debounceKey0 = true;
            debounceCounterKey0 = 15;  // key cooldown time to prohibit the wrong touch
        }
        if (debounceCounterKey0 > 0) {
            debounceCounterKey0--;
            if (debounceCounterKey0 == 0) {
                debounceKey0 = false;
            }
        }
        if (moveToPosition) {
            CamPos = glm::vec3(-19.144f, 15.6935f, -9.47934f);  // change the position of camera
            CamAlpha = -190.0f;  // around y axis to change the camera
            CamBeta = 0.0f;   // around the x aixs to change the vertical angles
            moveToPosition = false;
            std::cout << "Moved camera to position (-19.144, 15.6935, -9.47934)\n";
        }
        
        // 0 to change the position
        if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS && !debounceKey9) {
            moveToPosition9 = true;  // true
            debounceKey9 = true;
            debounceCounterKey9 = 15;  // key cooldown time to prohibit the wrong touch
        }
        if (debounceCounterKey9 > 0) {
            debounceCounterKey9--;
            if (debounceCounterKey9 == 0) {
                debounceKey9 = false;
            }
        }
        if (moveToPosition9) {
            CamPos = glm::vec3(-20.3933f, 4.68533f, 5.82232f);  // change the position of camera
            CamAlpha = -51.5f;  // around y axis to change the camera
            CamBeta = 0.0f;   // around the x aixs to change the vertical angles
            moveToPosition9 = false;
        }
        
        
        
        // when we turn off the flashlight, we set the enviroment light is  0
        glm::vec3 selectedLightColor = flashlightOn ? lightColorOptions[currentLightColor] : glm::vec3(0.0f);
        
        // uptate the flashlight's directions and positions
        glm::vec3 vertical = glm::rotate(glm::mat4(1.0f), CamBeta, glm::vec3(1, 0, 0)) * glm::vec4(0, 0, -1, 0); // vertical direction
        // The vertical and horizontal directions are separated, and only the Y-axis height is adjusted
        glm::vec3 spotlightDirection = glm::normalize(glm::vec3(forward.x, vertical.y, forward.z)); // mergin the vertical and horizontal directions
        glm::vec3 spotlightPosition = CamPos + forward * 1.5f; // keep the positions of flashlight in a horizontal position
        gubo.spotlightPos = spotlightPosition;
        gubo.spotlightDir = spotlightDirection;
        gubo.lightColor = glm::vec4(selectedLightColor, 1.0f); // use the colors we want
        gubo.spotlightCutoff = glm::cos(glm::radians(150.0f)); // the angles of light
        gubo.eyePos = CamPos;
        // uptate Descriptor Set
        DSGlobal.map(currentImage, &gubo, 0);
    
        // uptate the M0's Uniform —— park obj
        BlinnUniformBufferObject blinnUboM0{};
        glm::mat4 translationM0 = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, 0.0f)); // translation
        glm::mat4 rotationM0 = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0, 1, 0)); // rotate
        glm::mat4 scaleM0 = glm::scale(glm::mat4(1.0f), glm::vec3(0.020f)); // scale
        blinnUboM0.mMat = translationM0 * rotationM0 * scaleM0; // combination
        blinnUboM0.mvpMat = ViewPrj * blinnUboM0.mMat; // uptate MVP matrix
        blinnUboM0.nMat = glm::inverse(glm::transpose(blinnUboM0.mMat)); // Update the normal matrix
        DSM0.map(currentImage, &blinnUboM0, 0); // Map Uniform data to DSM0
        
        // uptate the M1's Uniform —— Tesla
        BlinnUniformBufferObject blinnUboM1{};
        glm::mat4 translationM1 = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.3f, 3.0f)); // translate
        glm::mat4 rotationXt = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0)); // rotate around x axis
        glm::mat4 rotationYt = glm::rotate(glm::mat4(1.0f), glm::radians(-135.0f), glm::vec3(0, 1, 0));  // rotate around y axis
        glm::mat4 rotationM1 = rotationYt * rotationXt; // rotate around y axis firstly and then x axis
        glm::mat4 scaleM1 = glm::scale(glm::mat4(1.0f), glm::vec3(0.0059f)); // scale
        blinnUboM1.mMat = translationM1 * rotationM1 * scaleM1; // combination
        blinnUboM1.mvpMat = ViewPrj * blinnUboM1.mMat; // uptate the MVP matrix
        blinnUboM1.nMat = glm::inverse(glm::transpose(blinnUboM1.mMat)); // update the normal matrix
        DSM1.map(currentImage, &blinnUboM1, 0); // map uniform date to DSM1
        
        // uptate M2's uniform —— Ford GT 2020
        BlinnUniformBufferObject blinnUboM2{};
        glm::mat4 translationM2 = glm::translate(glm::mat4(1.0f), glm::vec3(13.5f, 0.8f, -11.1f));
        glm::mat4 rotationXf = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
        glm::mat4 rotationYf = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0, 1, 0));
        glm::mat4 rotationM2 = rotationYf * rotationXf;
        glm::mat4 scaleM2 = glm::scale(glm::mat4(1.0f), glm::vec3(0.059f));
        blinnUboM2.mMat = translationM2 * rotationM2 * scaleM2;
        blinnUboM2.mvpMat = ViewPrj * blinnUboM2.mMat;
        blinnUboM2.nMat = glm::inverse(glm::transpose(blinnUboM2.mMat));
        DSM2.map(currentImage, &blinnUboM2, 0);
        
        // uptate M3's Uniform —— BenzAMGCLA
        BlinnUniformBufferObject blinnUboM3{};
        glm::mat4 translationM3 = glm::translate(glm::mat4(1.0f), glm::vec3(31.2917, 0.40, -2.54237));
        glm::mat4 rotationXM3 = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
        glm::mat4 rotationYM3 = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0, 1, 0));
        glm::mat4 rotationM3 = rotationYM3 * rotationXM3;
        glm::mat4 scaleM3 = glm::scale(glm::mat4(1.0f), glm::vec3(0.028f));
        blinnUboM3.mMat = translationM3 * rotationM3 * scaleM3;
        blinnUboM3.mvpMat = ViewPrj * blinnUboM3.mMat;
        blinnUboM3.nMat = glm::inverse(glm::transpose(blinnUboM3.mMat));
        DSM3.map(currentImage, &blinnUboM3, 0);
        
        // uptate M4's Uniform —— lambroghini2023
        BlinnUniformBufferObject blinnUboM4{};
        glm::mat4 translationM4 = glm::translate(glm::mat4(1.0f), glm::vec3(-14.15f, 0.32f, -24.57f));
        glm::mat4 rotationXM4 = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
        glm::mat4 rotationYM4 = glm::rotate(glm::mat4(1.0f), glm::radians(135.0f), glm::vec3(0, 1, 0));
        glm::mat4 rotationM4 = rotationYM4 * rotationXM4;
        glm::mat4 scaleM4 = glm::scale(glm::mat4(1.0f), glm::vec3(0.0024f));
        blinnUboM4.mMat = translationM4 * rotationM4 * scaleM4;
        blinnUboM4.mvpMat = ViewPrj * blinnUboM4.mMat;
        blinnUboM4.nMat = glm::inverse(glm::transpose(blinnUboM4.mMat));
        DSM4.map(currentImage, &blinnUboM4, 0);
        
        // uptate M5's Uniform —— Farrari
        BlinnUniformBufferObject blinnUboM5{};
        glm::mat4 translationM5 = glm::translate(glm::mat4(1.0f), glm::vec3(-20.4804, 0.44, -16.5909));
        glm::mat4 rotationXM5 = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
        glm::mat4 rotationYM5 = glm::rotate(glm::mat4(1.0f), glm::radians(135.0f), glm::vec3(0, 1, 0));
        glm::mat4 rotationM5 = rotationYM5 * rotationXM5;
        glm::mat4 scaleM5 = glm::scale(glm::mat4(1.0f), glm::vec3(0.062f));
        blinnUboM5.mMat = translationM5 * rotationM5 * scaleM5;
        blinnUboM5.mvpMat = ViewPrj * blinnUboM5.mMat;
        blinnUboM5.nMat = glm::inverse(glm::transpose(blinnUboM5.mMat));
        DSM5.map(currentImage, &blinnUboM5, 0);
        
        // uptate M6's Uniform —— landrover
        BlinnUniformBufferObject blinnUboM6{};
        glm::mat4 translationM6 = glm::translate(glm::mat4(1.0f), glm::vec3(12.72f, 0.4f, -21.05f));
        glm::mat4 rotationXM6 = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
        glm::mat4 rotationYM6 = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0, 1, 0));
        glm::mat4 rotationM6 = rotationYM6 * rotationXM6;
        glm::mat4 scaleM6 = glm::scale(glm::mat4(1.0f), glm::vec3(0.22f));
        blinnUboM6.mMat = translationM6 * rotationM6 * scaleM6;
        blinnUboM6.mvpMat = ViewPrj * blinnUboM6.mMat;
        blinnUboM6.nMat = glm::inverse(glm::transpose(blinnUboM6.mMat));
        DSM6.map(currentImage, &blinnUboM6, 0);
        
        // uptate M7's Uniform —— lambroghini2019
        BlinnUniformBufferObject blinnUboM7{};
        glm::mat4 translationM7 = glm::translate(glm::mat4(1.0f), glm::vec3(-2.63f, 0.5f, -37.20f));
        glm::mat4 rotationXM7 = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
        glm::mat4 rotationYM7 = glm::rotate(glm::mat4(1.0f), glm::radians(-135.0f), glm::vec3(0, 1, 0));
        glm::mat4 rotationM7 = rotationYM7 * rotationXM7;
        glm::mat4 scaleM7 = glm::scale(glm::mat4(1.0f), glm::vec3(0.048f));
        blinnUboM7.mMat = translationM7 * rotationM7 * scaleM7;
        blinnUboM7.mvpMat = ViewPrj * blinnUboM7.mMat;
        blinnUboM7.nMat = glm::inverse(glm::transpose(blinnUboM7.mMat));
        DSM7.map(currentImage, &blinnUboM7, 0);
        
        // uptate M8's Uniform —— Maserati 2024
        BlinnUniformBufferObject blinnUboM8{};
        glm::mat4 translationM8 = glm::translate(glm::mat4(1.0f), glm::vec3(20.68f, 0.47f, -20.01f));
        glm::mat4 rotationXM8 = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
        glm::mat4 rotationYM8 = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0, 1, 0));
        glm::mat4 rotationM8 = rotationYM8 * rotationXM8;
        glm::mat4 scaleM8 = glm::scale(glm::mat4(1.0f), glm::vec3(0.040f));
        blinnUboM8.mMat = translationM8 * rotationM8 * scaleM8;
        blinnUboM8.mvpMat = ViewPrj * blinnUboM8.mMat;
        blinnUboM8.nMat = glm::inverse(glm::transpose(blinnUboM8.mMat));
        DSM8.map(currentImage, &blinnUboM8, 0);
        
        // uptate M9's Uniform —— satellite1
        BlinnUniformBufferObject blinnUboM90{};
        glm::mat4 translationM90 = glm::translate(glm::mat4(1.0f), glm::vec3(-1.49622, 17.0, -17.0978));
        glm::mat4 rotationXM90 = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1, 0, 0));
        glm::mat4 rotationYM90 = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
        glm::mat4 rotationM90 = rotationYM90 * rotationXM90;
        glm::mat4 scaleM90 = glm::scale(glm::mat4(1.0f), glm::vec3(3.5f));
        blinnUboM90.mMat = translationM90 * rotationM90 * scaleM90;
        blinnUboM90.mvpMat = ViewPrj * blinnUboM90.mMat;
        blinnUboM90.nMat = glm::inverse(glm::transpose(blinnUboM90.mMat));
        DSM90.map(currentImage, &blinnUboM90, 0);
        
        // uptate M10's Uniform —— batman
        BlinnUniformBufferObject blinnUboM10{};
        glm::mat4 translationM10 = glm::translate(glm::mat4(1.0f), glm::vec3(-11.8208, 0.5, 5.08401));
        glm::mat4 rotationXM10 = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
        glm::mat4 rotationYM10 = glm::rotate(glm::mat4(1.0f), glm::radians(225.0f), glm::vec3(0, 1, 0));
        glm::mat4 rotationM10 = rotationYM10 * rotationXM10;
        glm::mat4 scaleM10 = glm::scale(glm::mat4(1.0f), glm::vec3(0.058f));
        blinnUboM10.mMat = translationM10 * rotationM10 * scaleM10;
        blinnUboM10.mvpMat = ViewPrj * blinnUboM10.mMat;
        blinnUboM10.nMat = glm::inverse(glm::transpose(blinnUboM10.mMat));
        DSM10.map(currentImage, &blinnUboM10, 0);
        
        // joker
        BlinnUniformBufferObject blinnUboM20{};
        glm::mat4 translationM20 = glm::translate(glm::mat4(1.0f), glm::vec3(-3.45205, 7.24445, 24.7622)); // Adjust position
        glm::mat4 rotationXM20 = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0));
        glm::mat4 rotationYM20 = glm::rotate(glm::mat4(1.0f), glm::radians(25.0f), glm::vec3(0, 1, 0));
        glm::mat4 rotationM20 = rotationYM20 * rotationXM20;
        glm::mat4 scaleM20 = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f)); // Scale
        blinnUboM20.mMat = translationM20 * rotationM20 * scaleM20;
        blinnUboM20.mvpMat = ViewPrj * blinnUboM20.mMat;
        blinnUboM20.nMat = glm::inverse(glm::transpose(blinnUboM20.mMat));
        DSM20.map(currentImage, &blinnUboM20, 0);
        
        // spaceshuttle M21
        // Updating M21 Uniform
        ToonUniformBufferObject blinnUboM21{};
        glm::mat4 translationM21 = glm::translate(glm::mat4(1.0f), glm::vec3(-9.46618, 11.60, 9.25385));
        glm::mat4 rotationXM21 = glm::rotate(glm::mat4(1.0f), glm::radians(270.0f), glm::vec3(1, 0, 0));
        glm::mat4 rotationYM21 = glm::rotate(glm::mat4(1.0f), glm::radians(-135.0f), glm::vec3(0, 1, 0));
        glm::mat4 rotationM21 = rotationYM21 * rotationXM21;
        glm::mat4 scaleM21 = glm::scale(glm::mat4(1.0f), glm::vec3(0.013f));
        blinnUboM21.mMat = translationM21 * rotationM21 * scaleM21;
        blinnUboM21.mvpMat = ViewPrj * blinnUboM21.mMat;
        blinnUboM21.nMat = glm::inverse(glm::transpose(blinnUboM21.mMat));
        DSM21.map(currentImage, &blinnUboM21, 0);
        
        // update skybox's Uniform
        skyBoxUniformBufferObject skyUbo{};
        // Translation matrix (can be used to adjust the position of the skybox)
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, -2.0f, 10.0f));
        // Scale matrix (resize the skybox)
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(100.0f)); // 100 x magnification
        // Remove the camera position effect from the view matrix
        glm::mat4 noTranslationView = glm::mat4(glm::mat3(Mv)); // remain rotate
        // combine MVP martix
        skyUbo.mvpMat = ViewPrj * noTranslationView * translationMatrix * scaleMatrix;
        // map
        DSskyBox.map(currentImage, &skyUbo, 0);
        
        // Updating the Sun's UniformBufferObject (Emit)
        EmitUniformBufferObject emitUbo{};
        glm::mat4 translationSun = glm::translate(glm::mat4(1.0f), glm::vec3(42.7539, 47.6142, 19.4343)); // Position Sun higher and farther
        glm::mat4 rotationSun = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0, 1, 0)); // Rotate Sun around Y-axis
        glm::mat4 scaleSun = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f)); // Scale Sun to 1.5x its original size
        emitUbo.mvpMat = ViewPrj * translationSun * rotationSun * scaleSun; // Combine transformations and apply View Projection
        DSsun.map(currentImage, &emitUbo, 0); // Map the updated UniformBufferObject for Sun
        
        
        // The M key controls the switching perspective
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !debounce) {
            debounce = true;
            curDebounce = 15;
            isPerspective = !isPerspective;
        }
        if (curDebounce > 0) {
            curDebounce--;
            if (curDebounce == 0) {
                debounce = false;
            }
        }
        
    }
};

// This is the main: probably you do not need to touch this!
int main() {
    E09 app;
    
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
