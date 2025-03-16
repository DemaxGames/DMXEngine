#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <mathc.h>
#include <vulkan/vulkan/vulkan.h>
#include <GLFW/glfw3.h> 
#define GLFW_INCLUDE_VULKAN // A really important part
                            // Include vulkan headers before GLFW headers
                            // Otherwise GLFW will not support Vulkan stuff

#define DEG_TO_RAD(deg) (deg * 0.0174533)

char* appName = "GTNH 2.0";
char* engineName = "DMXEngine";
uint32_t MAX_FRAMES_IN_FLIGHT = 1;



uint32_t FindMemType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    printf("ERROR: not finded memory type");
}

void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {
    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = NULL;
    bufferInfo.flags = 0;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(device, &bufferInfo, NULL, buffer);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemType(physicalDevice, memRequirements.memoryTypeBits, properties);

    vkAllocateMemory(device, &allocInfo, NULL, bufferMemory);

    vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
}

void createDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool) {
    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT;
    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = NULL;
    poolInfo.flags = 0;
    poolInfo.poolSizeCount = MAX_FRAMES_IN_FLIGHT;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;
    vkCreateDescriptorPool(device, &poolInfo, NULL, descriptorPool);
}

void createDescriptorSets(VkDevice device, VkDescriptorSetLayout* layout, VkDescriptorPool descriptorPool, VkDescriptorSet* descriptorSets) {
    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layout;
    vkAllocateDescriptorSets(device, &allocInfo, descriptorSets);
}

int main(int argc, char** argv){

    /***********Initializing GLFW Window***********/

    printf("Initializing GLFW\n");
    glfwInit();

    int width = 1080;
    int height = 720;
    

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(width, height, appName, NULL, NULL);

    
    /***********Creating App Info***********/

    printf("Initializing Vulkan\n");

    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = NULL;
    appInfo.pApplicationName = appName;
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = engineName;
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    /***********Creating Vulkan Instance Info***********/

    VkInstanceCreateInfo instanceInfo;
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = NULL;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
    uint32_t instLayerCount = 1;
    instanceInfo.enabledLayerCount = instLayerCount;
    char ppInstLayers[instLayerCount][VK_MAX_EXTENSION_NAME_SIZE];
    strcpy(ppInstLayers[0], "VK_LAYER_KHRONOS_validation");
    char *ppInstLayerNames[instLayerCount];
    for(int i = 0; i < instLayerCount; i++) ppInstLayerNames[i] = ppInstLayers[i];
    instanceInfo.ppEnabledLayerNames = (const char* const*)ppInstLayerNames;
    uint32_t instExtensionsCount = 0;
    const char* const* ppInstExtensionNames = glfwGetRequiredInstanceExtensions(&instExtensionsCount);
    instanceInfo.enabledExtensionCount = instExtensionsCount;
    instanceInfo.ppEnabledExtensionNames = ppInstExtensionNames;

    /***********Creating Vulkan Instance***********/

    printf("Creating Vulkan Instance\n");
    VkInstance instance;
    vkCreateInstance(&instanceInfo, NULL, &instance);
    
    /***********Setting Up Physical Device***********/

    printf("Creating Device\n");

    uint32_t physDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physDeviceCount, NULL);
    VkPhysicalDevice physDevices[physDeviceCount];
    vkEnumeratePhysicalDevices(instance, &physDeviceCount, physDevices);
    VkPhysicalDeviceProperties physDeviceProperties[physDeviceCount];
    uint32_t discreteGPUList[physDeviceCount];
    uint32_t discreteGPUCount = 0;
    uint32_t integratedGPUList[physDeviceCount];
    uint32_t integratedGPUCount = 0;
    VkPhysicalDeviceMemoryProperties physDeviceMemoryProperties[physDeviceCount];
    uint32_t physDeviceMemoryCount[physDeviceCount];
    VkDeviceSize physDeviceMemoryTotal[physDeviceCount];

    for(uint32_t i = 0; i < physDeviceCount; i++){
        vkGetPhysicalDeviceProperties(physDevices[i], &physDeviceProperties[i]);
        if(physDeviceProperties[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
            discreteGPUList[discreteGPUCount] = i;
            discreteGPUCount++;
        }else if(physDeviceProperties[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU){
            integratedGPUList[discreteGPUCount] = i;
            integratedGPUCount++;
        }

        vkGetPhysicalDeviceMemoryProperties(physDevices[i], &physDeviceMemoryProperties[i]);
        physDeviceMemoryCount[i] = physDeviceMemoryProperties[i].memoryHeapCount;
        physDeviceMemoryTotal[i] = 0;
        for(uint32_t j = 0; j < physDeviceMemoryCount[i]; j++){
            physDeviceMemoryTotal[i] += physDeviceMemoryProperties[i].memoryHeaps[j].size;
        }
    }

    VkDeviceSize maxMemSize = 0;
    uint32_t physDeviceBestIndex = 0;

    if(discreteGPUCount != 0){
        for(uint32_t i = 0; i < discreteGPUCount; i++){
            if(physDeviceMemoryTotal[i] > maxMemSize){
                physDeviceBestIndex = discreteGPUList[i];
                maxMemSize = physDeviceMemoryTotal[i];
            }
        }
    } else if(integratedGPUCount != 0){
        for(uint32_t i = 0; i < integratedGPUCount; i++){
            if(physDeviceMemoryTotal[i] > maxMemSize){
                physDeviceBestIndex = integratedGPUList[i];
                maxMemSize = physDeviceMemoryTotal[i];
            }
        }
    }
    

    printf("Devices found (%d): \n", physDeviceCount);

    for(uint32_t i = 0; i < physDeviceCount; i++){
        printf("\tDevice name: %s:\n", physDeviceProperties[i].deviceName);
        if(physDeviceProperties[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU){
            printf("\t\tDevice type: Integrated GPU\n");
        } else if(physDeviceProperties[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
            printf("\t\tDevice type: Discrete GPU\n");
        }
        unsigned int GBs = (physDeviceMemoryTotal[i]);
        GBs /= 1024. * 1024. * 1024.;
        printf("\t\tDevice memory total: %llu (%u GB)\n", (int)physDeviceMemoryTotal[i], GBs);
    }

    VkPhysicalDevice* physDevice = &physDevices[physDeviceBestIndex];
    uint32_t qfPropertiesCount = 0; // qf stands for queue family
    vkGetPhysicalDeviceQueueFamilyProperties(*physDevice, &qfPropertiesCount, NULL);
    VkQueueFamilyProperties qfProperties[qfPropertiesCount];
    vkGetPhysicalDeviceQueueFamilyProperties(*physDevice, &qfPropertiesCount, qfProperties);
    
    uint32_t qfqCount[qfPropertiesCount]; // qfQ stands for queue family queue

    for(uint32_t i = 0; i < qfPropertiesCount; i++){
        qfqCount[i] = qfProperties[i].queueCount;
    }

    /***********Creating Logical Device***********/

    VkDeviceQueueCreateInfo deviceQInfos[qfPropertiesCount];
    float qPriority[20];
    for(uint32_t i = 0; i < 20; i++) qPriority[i] = 1.0;

    for(uint32_t i = 0; i < qfPropertiesCount; i++){

        deviceQInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQInfos[i].pNext = NULL;
        deviceQInfos[i].flags = 0;
        deviceQInfos[i].queueFamilyIndex = i;
        deviceQInfos[i].queueCount = qfProperties[i].queueCount;
        deviceQInfos[i].pQueuePriorities = qPriority;
        //printf("\tDebug: pQueuePriorities[%d] = %f at %f\n", i, *deviceQInfos[i].pQueuePriorities, deviceQInfos[i].pQueuePriorities);
    }

    printf("using %d queue families\n", qfPropertiesCount);

    VkDeviceCreateInfo deviceInfo;
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = NULL;
    deviceInfo.flags = 0;
    deviceInfo.queueCreateInfoCount = qfPropertiesCount;
    deviceInfo.pQueueCreateInfos = deviceQInfos;
    deviceInfo.enabledLayerCount = 0;
    deviceInfo.ppEnabledLayerNames = NULL;

    uint32_t deviceExtCount = 1; // device extension count
    deviceInfo.enabledExtensionCount = deviceExtCount;
    char ppDeviceExts[deviceExtCount][VK_MAX_EXTENSION_NAME_SIZE];
    strcpy(ppDeviceExts[0], "VK_KHR_swapchain");
    char* ppDeviceExtNames[deviceExtCount];

    for(uint32_t i = 0; i < deviceExtCount; i++){
        ppDeviceExtNames[i] = ppDeviceExts[i];
    }

    deviceInfo.ppEnabledExtensionNames = (const char* const*)ppDeviceExtNames;

    VkPhysicalDeviceFeatures physDeviceFeatures;
    vkGetPhysicalDeviceFeatures(*physDevice, &physDeviceFeatures);
    deviceInfo.pEnabledFeatures = &physDeviceFeatures;

    VkDevice device;
    vkCreateDevice(*physDevice, &deviceInfo, NULL, &device);
    printf("Logical Device Created\n");

    /***********Selecting Queue***********/

    uint32_t qfGraphicsCount = 0;
    uint32_t qfGraphicsList[qfPropertiesCount];

    for(uint32_t i = 0; i < qfPropertiesCount; i++){
        if((qfProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0){
            qfGraphicsList[qfGraphicsCount] = i;
            qfGraphicsCount++;
        }
    }

    uint32_t maxqCount = 0;
    uint32_t qfBestIndex = 0;

    for(uint32_t i = 0; i < qfGraphicsCount; i++){
        if(qfProperties[qfGraphicsList[i]].queueCount > maxqCount){
            qfBestIndex = qfGraphicsList[i];
        }
    }

    printf("Best queue family index: %d\n", qfBestIndex);

    VkQueue qGraph, qPress;
    vkGetDeviceQueue(device, qfBestIndex, 0, &qGraph);
    int8_t singleQueue = 1;
    if(qfProperties[qfBestIndex].queueCount < 2){
        vkGetDeviceQueue(device, qfBestIndex, 0, &qPress);
        printf("Using single queue for drawing\n");
    } else {
        singleQueue = 0;
        vkGetDeviceQueue(device, qfBestIndex, 1, &qPress);
        printf("Using double queues for drawing\n");
    }

    /***********Creating Vulkan Surface For GLFW***********/

    VkSurfaceKHR surface;
    glfwCreateWindowSurface(instance, window, NULL, &surface);
    printf("Surface created\n");

    VkBool32 physSurfaceSupported;
    vkGetPhysicalDeviceSurfaceSupportKHR(*physDevice, qfBestIndex, surface, &physSurfaceSupported);
    if(physSurfaceSupported == VK_TRUE) printf("Surface supported\n");
    else printf("Warning: Surface unsupported!\n");

    VkSurfaceCapabilitiesKHR surfaceCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*physDevice, surface, &surfaceCaps);
    int8_t extSuitable = 1;
    uint32_t window_w, window_h;
    glfwGetFramebufferSize(window, &window_w, &window_h);
    VkExtent2D actualExt;
    actualExt.width = window_w;
    actualExt.height = window_h;
    if(surfaceCaps.currentExtent.width != window_w ||
        surfaceCaps.currentExtent.height != window_h){
        extSuitable = 0;
        printf("Actual extent size doesn't match framebuffers, resizing...\n");
        actualExt.width = window_w > surfaceCaps.maxImageExtent.width ? surfaceCaps.maxImageExtent.width:window_w; // sorry for this crappy one
        actualExt.width = window_w > surfaceCaps.minImageExtent.width ? surfaceCaps.minImageExtent.width:window_w; // and this one
        actualExt.height = window_w > surfaceCaps.maxImageExtent.height ? surfaceCaps.maxImageExtent.height:window_w; // and this one
        actualExt.height = window_w > surfaceCaps.minImageExtent.height ? surfaceCaps.minImageExtent.height:window_w; // and this one
    }

    uint32_t surfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(*physDevice, surface, &surfaceFormatCount, NULL);
    VkSurfaceFormatKHR surfaceFormats[surfaceFormatCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(*physDevice, surface, &surfaceFormatCount, surfaceFormats);
    printf("Fetched %d surface formats:\n", surfaceFormatCount);
    for(uint32_t i = 0; i < surfaceFormatCount; i++){
        printf("\tFormat: %d colorspace %x\n", surfaceFormats[i].format, surfaceFormats[i].colorSpace);
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(*physDevice, surface, &presentModeCount, NULL);
    VkPresentModeKHR presentModes[presentModeCount];
    vkGetPhysicalDeviceSurfacePresentModesKHR(*physDevice, surface, &presentModeCount, presentModes);
    printf("Fetched %d present modes\n", presentModeCount);
    uint8_t mailboxModeSupported = 0;
    for(uint32_t i; i < presentModeCount; i++){
        printf("present mode: %d\n", presentModes[i]);
        if(presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR){
            printf("mailbox present mode supported\n");
            mailboxModeSupported = 1;
        }
    }

    /***********Creating Swapchain***********/

    VkSwapchainCreateInfoKHR swapchainInfo;
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.pNext = NULL;
    swapchainInfo.flags = 0;
    swapchainInfo.surface = surface;
    swapchainInfo.minImageCount = surfaceCaps.minImageCount + 1;
    swapchainInfo.imageFormat = surfaceFormats[0].format;
    swapchainInfo.imageColorSpace = surfaceFormats[0].colorSpace;
    swapchainInfo.imageExtent = extSuitable ? surfaceCaps.currentExtent : actualExt;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode = singleQueue ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
    swapchainInfo.queueFamilyIndexCount = singleQueue ? 0 : 2;
    uint32_t qfIndices[2] = {0, 1};
    swapchainInfo.pQueueFamilyIndices = singleQueue ? NULL : qfIndices;
    swapchainInfo.preTransform = surfaceCaps.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    //swapchainInfo.presentMode = mailboxModeSupported ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_FIFO_KHR;
    swapchainInfo.clipped = VK_FALSE;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain;
    vkCreateSwapchainKHR(device, &swapchainInfo, NULL, &swapchain);
    printf("Swapchain created\n");

    /***********Fetching Image From Swapchain***********/

    uint32_t swapImageCount = 0;
    vkGetSwapchainImagesKHR(device, swapchain, &swapImageCount, NULL);
    VkImage swapImages[swapImageCount];
    vkGetSwapchainImagesKHR(device, swapchain, &swapImageCount, swapImages);
    printf("%d images fetched from swapchain\n", swapImageCount);

    /***********Creating Image View***********/

    VkImageView imageViews[swapImageCount];
    VkImageViewCreateInfo imageViewInfos[swapImageCount];

    VkComponentMapping imageViewRGBAComp;
    imageViewRGBAComp.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewRGBAComp.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewRGBAComp.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewRGBAComp.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    VkImageSubresourceRange imageViewSubres;
    imageViewSubres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewSubres.baseMipLevel = 0;
    imageViewSubres.levelCount = 1;
    imageViewSubres.baseArrayLayer = 0;
    imageViewSubres.layerCount = swapchainInfo.imageArrayLayers;

    for(uint32_t i = 0; i < swapImageCount; i++){
        imageViewInfos[i].sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfos[i].pNext = NULL;
        imageViewInfos[i].flags = 0;
        imageViewInfos[i].image = swapImages[i];
        imageViewInfos[i].viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfos[i].format = surfaceFormats[0].format;
        imageViewInfos[i].components = imageViewRGBAComp;
        imageViewInfos[i].subresourceRange = imageViewSubres;
        vkCreateImageView(device, &imageViewInfos[i], NULL, &imageViews[i]);
        printf("Image view %d created\n", i);
    }

    /***********Creating Render Pass***********/

    VkAttachmentDescription attachDescription;
    attachDescription.flags = 0;
    attachDescription.format = swapchainInfo.imageFormat;
    attachDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference attachReference;
    attachReference.attachment = 0;
    attachReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription SubpassDescription;
    SubpassDescription.flags = 0;
    SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    SubpassDescription.inputAttachmentCount = 0;
    SubpassDescription.pInputAttachments = NULL;
    SubpassDescription.colorAttachmentCount = 1;
    SubpassDescription.pColorAttachments = &attachReference;
    SubpassDescription.pResolveAttachments = NULL;
    SubpassDescription.pDepthStencilAttachment = NULL;
    SubpassDescription.preserveAttachmentCount = 0;
    SubpassDescription.pPreserveAttachments = NULL;

    VkSubpassDependency SubpassDependency;
    SubpassDependency.dependencyFlags = 0;
    SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    SubpassDependency.dstSubpass = 0;
    SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    SubpassDependency.srcAccessMask = 0;
    SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = NULL;
    renderPassInfo.flags = 0;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &attachDescription;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &SubpassDescription;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &SubpassDependency;

    VkRenderPass renderPass;
    vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass);
    printf("Renderpass created\n");

    /***********Creating Pipeline***********/

    /****Loading shaders****/

    FILE* fpVert = NULL;
    fpVert = fopen("vertex.spv", "rb+");
    FILE* fpFrag = NULL;
    fpFrag = fopen("fragment.spv", "rb+");
    int8_t shaderLoaded = 0;
    if(fpVert != NULL && fpFrag != NULL){
        shaderLoaded = 1;
    } else {
        printf("ERROR: can't finde shader.vert or shader.frag");
        return -1;
    }
    fseek(fpVert, 0, SEEK_END);
    fseek(fpFrag, 0, SEEK_END);
    uint32_t vertSize = ftell(fpVert);
    uint32_t fragSize = ftell(fpFrag);
    char* vertexShaderCode = malloc(vertSize*sizeof(char));
    char* FragmentShaderCode = malloc(fragSize*sizeof(char));
    rewind(fpVert);
    rewind(fpFrag);
    fread(vertexShaderCode, 1, vertSize, fpVert);
    fread(FragmentShaderCode, 1, fragSize, fpFrag);

    /****Creating Shader Modules****/
    VkShaderModuleCreateInfo vertShaderModuleInfo;
    vertShaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertShaderModuleInfo.pNext = NULL;
    vertShaderModuleInfo.flags = 0;
    vertShaderModuleInfo.pCode = (const uint32_t*) vertexShaderCode;
    vertShaderModuleInfo.codeSize = vertSize;

    VkShaderModuleCreateInfo fragShaderModuleInfo;
    fragShaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragShaderModuleInfo.pNext = NULL;
    fragShaderModuleInfo.flags = 0;
    fragShaderModuleInfo.pCode = (const uint32_t*) FragmentShaderCode;
    fragShaderModuleInfo.codeSize = fragSize;

    VkShaderModule vertShaderModule, fragShaderModule;
    vkCreateShaderModule(device, &vertShaderModuleInfo, NULL, &vertShaderModule);
    printf("Vertex shader loaded\n");
    vkCreateShaderModule(device, &fragShaderModuleInfo, NULL, &fragShaderModule);
    printf("Fragment shader loaded\n");

    /****Shader In Pipeline****/

    VkPipelineShaderStageCreateInfo vertShaderStageInfo, fragShaderStageInfo;
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.pNext = NULL;
    vertShaderStageInfo.flags = 0;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    char vertEntry[VK_MAX_EXTENSION_NAME_SIZE];
    strcpy(vertEntry, "main");
    vertShaderStageInfo.pName = vertEntry;
    vertShaderStageInfo.pSpecializationInfo = NULL;
    
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.pNext = NULL;
    fragShaderStageInfo.flags = 0;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    char fragEntry[VK_MAX_EXTENSION_NAME_SIZE];
    strcpy(fragEntry, "main");
    fragShaderStageInfo.pName = fragEntry;
    fragShaderStageInfo.pSpecializationInfo = NULL;

    VkPipelineShaderStageCreateInfo shaderStageInfos[2];
    shaderStageInfos[0] = vertShaderStageInfo;
    shaderStageInfos[1] = fragShaderStageInfo;

    float verticies[] = {
        0.0, -0.5,
	    0.5, 0.5,
	    -0.5,0.5
    };
    uint32_t vertexCount = 3;

    VkBufferCreateInfo vertexBufferInfo;
    vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferInfo.pNext = NULL;
    vertexBufferInfo.flags = 0;
    vertexBufferInfo.size = sizeof(verticies[0]) * 2 * vertexCount;
    vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vertexBufferInfo.queueFamilyIndexCount = 0;
    vertexBufferInfo.pQueueFamilyIndices = NULL;

    VkBuffer vertexBuffer;
    vkCreateBuffer(device, &vertexBufferInfo, NULL, &vertexBuffer);
    VkDeviceMemory vertexBufferMemory;
    printf("Vertex Buffer Created");

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(device, vertexBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = FindMemType(*physDevice, memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkAllocateMemory(device, &allocInfo, NULL, &vertexBufferMemory);
    vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);
    
    VkMappedMemoryRange memFlush;
    memFlush.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    memFlush.pNext = 0;
    memFlush.memory = vertexBufferMemory;
    memFlush.size = vertexBufferInfo.size;
    memFlush.offset = 0;

    void* data;
    vkMapMemory(device, vertexBufferMemory, 0, vertexBufferInfo.size, 0, &data);
    memcpy(data, verticies, (size_t) vertexBufferInfo.size);
    //vkFlushMappedMemoryRanges(device, 1, &memFlush);
    vkUnmapMemory(device, vertexBufferMemory);

    VkVertexInputBindingDescription vertexBufferDescription;
    vertexBufferDescription.binding = 0;
    vertexBufferDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertexBufferDescription.stride = sizeof(float) * 2;

    VkVertexInputAttributeDescription vertexAttributeDescription;
    vertexAttributeDescription.binding = 0;
    vertexAttributeDescription.location = 0;
    vertexAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
    vertexAttributeDescription.offset = 0;

    VkDescriptorSetLayoutBinding uboLayoutBinding;
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = NULL;

    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;
    void* uniformBufferMapped;
    struct {
        struct mat4 rotation;
    } UniformBufferObject;
    mat4_identity(&UniformBufferObject.rotation.m11);

    VkDeviceSize bufferSize = sizeof(float) * 4 * 4;
    createBuffer(device, *physDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uniformBuffer, &uniformBufferMemory);
    vkMapMemory(device, uniformBufferMemory, 0, bufferSize, 0, &uniformBufferMapped);
    memcpy(uniformBufferMapped, &UniformBufferObject, sizeof(float) * 4 * 4);
    
    VkDescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = NULL;
    layoutInfo.flags = 0;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    VkDescriptorSetLayout descriptorSetLayout;
    vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descriptorSetLayout);

    VkDescriptorPool descriptorPool;
    createDescriptorPool(device, &descriptorPool);
    VkDescriptorSet descriptorSet;
    createDescriptorSets(device, &descriptorSetLayout, descriptorPool, &descriptorSet);
    VkDescriptorBufferInfo descriptorBufferInfo;
    descriptorBufferInfo.buffer = uniformBuffer;
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = bufferSize;

    VkWriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = NULL;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
    writeDescriptorSet.pImageInfo = NULL;
    writeDescriptorSet.pTexelBufferView = NULL;

    vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, NULL);
    
    VkPipelineVertexInputStateCreateInfo vertInputInfo;
    vertInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertInputInfo.pNext = NULL;
    vertInputInfo.flags = 0;
    vertInputInfo.vertexBindingDescriptionCount = 1;
    vertInputInfo.pVertexBindingDescriptions = &vertexBufferDescription;
    vertInputInfo.vertexAttributeDescriptionCount = 1;
    vertInputInfo.pVertexAttributeDescriptions = &vertexAttributeDescription;

    VkPipelineInputAssemblyStateCreateInfo inputAsmInfo;
    inputAsmInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAsmInfo.pNext = NULL;
    inputAsmInfo.flags = 0;
    inputAsmInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAsmInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapchainInfo.imageExtent.width;
    viewport.height = (float)swapchainInfo.imageExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 2.0f;
    
    VkRect2D scissor;
    VkOffset2D scissorOffset;
    scissorOffset.x = 0;
    scissorOffset.y = 0;
    scissor.offset = scissorOffset;
    scissor.extent = swapchainInfo.imageExtent;

    VkPipelineViewportStateCreateInfo pipelineViewportInfo;
    pipelineViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipelineViewportInfo.pNext = NULL;
    pipelineViewportInfo.flags = 0;
    pipelineViewportInfo.viewportCount = 1;
    pipelineViewportInfo.pViewports = &viewport;
    pipelineViewportInfo.scissorCount = 1;
    pipelineViewportInfo.pScissors = &scissor;
    
    VkPipelineRasterizationStateCreateInfo rasterInfo;
    rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterInfo.pNext = NULL;
    rasterInfo.flags = 0;
    rasterInfo.depthClampEnable = VK_TRUE;
    rasterInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterInfo.cullMode = VK_CULL_MODE_NONE;
    rasterInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterInfo.depthBiasEnable = VK_FALSE;
    rasterInfo.depthBiasConstantFactor = 0.0f;
    rasterInfo.depthBiasClamp = 0.0f;
    rasterInfo.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo pipelineMultisample;
    pipelineMultisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineMultisample.pNext = NULL;
    pipelineMultisample.flags = 0;
    pipelineMultisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineMultisample.sampleShadingEnable = VK_FALSE;
    pipelineMultisample.minSampleShading = 1.0f;
    pipelineMultisample.pSampleMask = NULL;
    pipelineMultisample.alphaToCoverageEnable = VK_FALSE;
    pipelineMultisample.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttach;
    colorBlendAttach.blendEnable = VK_FALSE;
    colorBlendAttach.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttach.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttach.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttach.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                     VK_COLOR_COMPONENT_G_BIT |
                                     VK_COLOR_COMPONENT_B_BIT |
                                     VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.pNext = NULL;
    colorBlendInfo.flags = 0;
    colorBlendInfo.logicOpEnable = VK_FALSE;
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &colorBlendAttach;
    for(uint32_t i = 0; i < 4; i++){
        colorBlendInfo.blendConstants[i] = 0.0f;
    }
    
    

    

    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = NULL;
    pipelineLayoutInfo.flags = 0;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = NULL;
    
    
    VkPipelineLayout pipelineLayout;
    vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout);

    /****Creating Pipeline****/

    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = NULL;
    pipelineInfo.flags = 0;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStageInfos;
    pipelineInfo.pVertexInputState = &vertInputInfo;
    pipelineInfo.pInputAssemblyState = & inputAsmInfo;
    pipelineInfo.pTessellationState = NULL;
    pipelineInfo.pViewportState = &pipelineViewportInfo;
    pipelineInfo.pRasterizationState = &rasterInfo;
    pipelineInfo.pMultisampleState = &pipelineMultisample;
    pipelineInfo.pDepthStencilState = NULL;
    pipelineInfo.pColorBlendState = &colorBlendInfo;
    pipelineInfo.pDynamicState = NULL;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VkPipeline pipeline;
    vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &pipeline);
    printf("Pipeline created\n");

    /****Creating Framebuffer****/

    VkFramebufferCreateInfo framebufferInfo[swapImageCount];
    VkFramebuffer framebuffers[swapImageCount];

    VkImageView imageAttachs[swapImageCount];
    for(uint32_t i = 0; i < swapImageCount; i++){
        imageAttachs[i] = imageViews[i];
        framebufferInfo[i].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo[i].pNext = NULL;
        framebufferInfo[i].flags = 0;
        framebufferInfo[i].renderPass = renderPass;
        framebufferInfo[i].attachmentCount = 1;
        framebufferInfo[i].pAttachments = &(imageAttachs[i]);
        framebufferInfo[i].width = swapchainInfo.imageExtent.width;
        framebufferInfo[i].height = swapchainInfo.imageExtent.height;
        framebufferInfo[i].layers = 1;

        vkCreateFramebuffer(device, &framebufferInfo[i], NULL, &framebuffers[i]);
        printf("Framebuffer %d created\n", i);
    }

    /****Creating CommandBuffer****/
    
    VkCommandPoolCreateInfo cmdPoolInfo;
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.pNext = NULL;
    cmdPoolInfo.flags = 0;
    cmdPoolInfo.queueFamilyIndex = qfBestIndex;
    
    VkCommandPool cmdPool;
    vkCreateCommandPool(device, &cmdPoolInfo, NULL, &cmdPool);

    VkCommandBufferAllocateInfo cmdBufferInfo;
    cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufferInfo.pNext = NULL;
    cmdBufferInfo.commandPool = cmdPool;
    cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferInfo.commandBufferCount = swapImageCount;

    VkCommandBuffer cmdBuffers[swapImageCount];

    vkAllocateCommandBuffers(device, &cmdBufferInfo, cmdBuffers);
    printf("Command buffers created (%d)\n", swapImageCount);

    /****Render Preparation****/

    VkCommandBufferBeginInfo cmdBufferBeginInfos[swapImageCount];
    VkRenderPassBeginInfo renderPassBeginInfos[swapImageCount];
    VkRect2D renderArea;
    renderArea.offset.x = 0;
    renderArea.offset.y = 0;
    renderArea.extent = swapchainInfo.imageExtent;
    VkClearValue clearValue = {0.2f, 0.3f, 0.5f, 0.0f};

    for(uint32_t i = 0; i < swapImageCount; i++){
        cmdBufferBeginInfos[i].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufferBeginInfos[i].pNext = NULL;
        cmdBufferBeginInfos[i].flags = 0;
        cmdBufferBeginInfos[i].pInheritanceInfo = NULL;
        
        renderPassBeginInfos[i].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfos[i].pNext = NULL;
        renderPassBeginInfos[i].renderPass = renderPass;
        renderPassBeginInfos[i].renderArea = renderArea;
        renderPassBeginInfos[i].framebuffer = framebuffers[i];
        renderPassBeginInfos[i].clearValueCount = 1;
        renderPassBeginInfos[i].pClearValues = &clearValue;
        
        vkBeginCommandBuffer(cmdBuffers[i], &cmdBufferBeginInfos[i]);
        vkCmdBeginRenderPass(cmdBuffers[i], &renderPassBeginInfos[i], VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmdBuffers[i], 0, 1, vertexBuffers, offsets);
        
        vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

        vkCmdDraw(cmdBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(cmdBuffers[i]);

        vkEndCommandBuffer(cmdBuffers[i]);

        printf("Command buffer drawing recordered (%d)\n", i);
    }

    /****Creating Semaphore****/

    uint32_t maxFrames = 2;
    VkSemaphore semaphoreImgAvl[maxFrames];
    VkSemaphore semaphoreRendFin[maxFrames];
    
    VkFence fence[maxFrames];

    VkSemaphoreCreateInfo semaphoreInfo;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = NULL;
    semaphoreInfo.flags = 0;

    VkFenceCreateInfo fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = NULL;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(uint32_t i = 0; i < maxFrames; i++){
        vkCreateSemaphore(device, &semaphoreInfo, NULL, &(semaphoreImgAvl[i]));
        vkCreateSemaphore(device, &semaphoreInfo, NULL, &(semaphoreRendFin[i]));
        
        vkCreateFence(device, &fenceInfo, NULL, &(fence[i]));
    }

    printf("Semaphores and fences created\n");

    VkFence fenceImg[swapImageCount];
    for(uint32_t i = 0; i < swapImageCount; i++){
        fenceImg[i] = VK_NULL_HANDLE;
    }

    /***********Main Loop***********/

    VkPipelineStageFlags waitStages[1];
    waitStages[0] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo subInfo;
    subInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    subInfo.pNext = NULL;
    subInfo.commandBufferCount = 1;
    subInfo.waitSemaphoreCount = 1;


    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    
    VkSwapchainKHR swap[1];
    swap[0] = swapchain;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &(swap[0]);
    presentInfo.pResults = NULL;

    

    uint32_t frame = 0;
    float time = DEG_TO_RAD(-360);
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        vkWaitForFences(device, 1, &(fence[frame]), VK_TRUE, UINT64_MAX);
        uint32_t imgIndex = 0;
        vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphoreImgAvl[frame], VK_NULL_HANDLE, &imgIndex);

        if(fenceImg[imgIndex] != VK_NULL_HANDLE){
            vkWaitForFences(device, 1, &(fenceImg[imgIndex]), VK_TRUE, UINT64_MAX);
        }

        fenceImg[imgIndex] = fence[frame];
        
        VkSemaphore semaphoreWait[1];
        semaphoreWait[0] = semaphoreImgAvl[frame];

        subInfo.pWaitSemaphores = &(semaphoreWait[0]);
        subInfo.pWaitDstStageMask = &(waitStages[0]);
        subInfo.pCommandBuffers = &(cmdBuffers[imgIndex]);

        VkSemaphore sempSig[1];
        sempSig[0] = semaphoreRendFin[frame];
        
        subInfo.signalSemaphoreCount = 1;
        subInfo.pSignalSemaphores = &(sempSig[0]);

        vkResetFences(device, 1, &(fence[frame]));
        vkQueueSubmit(qGraph, 1, &subInfo, fence[frame]);
        
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &(sempSig[0]);
        
        presentInfo.pImageIndices = &imgIndex;
        
        vkQueuePresentKHR(qPress, &presentInfo);

        mat4_rotation_y(&UniformBufferObject.rotation.m11, time);
        memcpy(uniformBufferMapped, &UniformBufferObject, sizeof(float) * 4 * 4);
        
        frame = (frame + 1) % maxFrames;
        time = time + 0.005;
    }

    vkDeviceWaitIdle(device);
	printf("command buffers finished.\n");

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void VulkanInit(){

}