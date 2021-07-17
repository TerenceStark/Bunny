#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include "shader.h"
#include "camera.h"
#include "parser.h"

#define INF 1.0e10
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_click_callback(GLFWwindow *window, int button, int action, int mods);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void do_movement();

// ����
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// ���
Camera camera(glm::vec3(0.0f, 0.0f, 30.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// ʱ�����
float deltaTime = 0.0f; 
float lastFrame = 0.0f;

// ���Ӳ���
glm::mat4 modelBunny = glm::mat4(1.0f);	//�Ľ׵�λ��
GLuint vertices_size, indices_size;	//�޷������� �����Լ���Ƭsize

// ���ղ���
glm::vec3 PointlightPos(0.0f, 15.0f, 0.0f);	//��ά���� ����

//������
GLfloat mouseX = SCR_WIDTH / 2.0;
GLfloat mouseY = SCR_HEIGHT / 2.0;
GLuint selectedPointIndice = 0; // max

const int MAXPOINT = 40000;
const int MAXINDEX = 70000;

const char* normalFile = "bunny_normal.ply2";
const char *SPLIT = "--------------------------------------------------------------";
GLfloat vertices[MAXPOINT*6];
GLuint indices[MAXINDEX*3];

bool keys[1024];	//��ȡ����
bool isAttenuation = false;
bool isFlashlight = false;
bool cursorDisabled = true;

void description(){
    std::cout << SPLIT << std::endl;
    std::cout << "Starting GLFW context, OpenGL 3.3\n";
    std::cout << "=: ���Դ˥��\n";
    std::cout << "P: ���տ���\n";
    std::cout << "A: �������.\n";
    std::cout << "D: �������.\n";
    std::cout << "W: �����ǰ\n";
    std::cout << "S: ������.\n";
    std::cout << "�����<-: ��������.\n";
    std::cout << "�����->: ��������.\n";
    std::cout << "J: ����������ת.\n";
    std::cout << "L: ����������ת.\n";
    std::cout << "I: ������ǰ��ת.\n";
    std::cout << "K: ���������ת\n";
    std::cout << "X: ���ӷŴ�\n";
    std::cout << "Z: ������С.\n";
    std::cout << "Tab: �߿�ģʽ�����ù�꣩| Ĭ��ģʽ�����ù�꣩\n";
    std::cout << SPLIT << std::endl;
}

//���ڳ�ʼ������
GLFWwindow* init(){
    glfwInit();//��ʼ��glfw��
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//���汾
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//�ΰ汾
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw ���ڴ���
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ZhangQi_Stanford bunny", NULL, NULL);
	
	//������ڴ���ʧ�ܣ�����ʧ����Ϣ��
    if (window == NULL){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();	//�ͷ�������Դ
        exit(EXIT_SUCCESS);
    }

    glfwMakeContextCurrent(window);	// ���õ�ǰ�Ĵ��������� 
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);	//ע�ắ�����ú���������ע���������Ӳ���Ļص�����
    glfwSetKeyCallback(window, key_callback);	//���ð����ص�����
    glfwSetCursorPosCallback(window, mouse_callback);	//���ù��ص�����
    glfwSetMouseButtonCallback(window, mouse_click_callback);	//������갴���ص�����
    glfwSetScrollCallback(window, scroll_callback);		//���ù��ֻص�����

    // ��GLFW�������
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: ��������openGL����ָ��
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(EXIT_SUCCESS);
    }

    // ����opengl��ȫ��״̬
    glEnable(GL_DEPTH_TEST);
    return window;
}
int main(){
    // glfw: ��ʼ������������ز���
	//����˵��
    description();

	//���ô��ڳ�ʼ������
    GLFWwindow* window = init();

    //������������ɫ��
    Shader lightingShader("bunny.vs", "bunny.fs");	//vs�ļ����������� fs�ļ�������������
    Shader lampShader("lamp.vs", "lamp.fs");		//����ɫ��
    Shader selectShader("select.vs", "select.fs");	//ʰȡ��ɫ��

    // ���ö������ݣ��ͻ������������ö�������
    //����ply2�ļ�
    parseNormal(normalFile, vertices, indices, vertices_size, indices_size);

    ///*
    //test code:
       for (int i = 0; i < 3; i++)
    {
        std::cout << vertices[6*i] << " " << vertices[6*i+1] << " " << vertices[6*i+2] << " " << vertices[6*i+3] << " " << vertices[6*i+4] << " " << vertices[6*i+5] << std::endl;
    }
   // */
 
   
	//�ƹⶥ������
    float lightVertices[] = {
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, 
         0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, 
        -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 

        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, 
         0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f, 

        -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, 
        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 
        -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, 

         0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f, 
         0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f, 

        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, 
        -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, 

        -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, 
         0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f
    };

    // ��һ�����������ӵ�VAO(��VBO)
    unsigned int bunnyVAO, bunnyVBO, bunnyEBO;
    glGenVertexArrays(1, &bunnyVAO);	//����һ��VAO����
    glGenBuffers(1, &bunnyVBO);	//�����Դ�ռ䣬����VBO��ID
    glGenBuffers(1, &bunnyEBO);	//���������������

    glBindVertexArray(bunnyVAO);		//��VAO����
        glBindBuffer(GL_ARRAY_BUFFER, bunnyVBO);	//��VBO
        glBufferData(GL_ARRAY_BUFFER, 6*vertices_size*sizeof(GLfloat), vertices, GL_STATIC_DRAW);//���û�����Ķ������ݴ��䵽�󶨵��Դ滺������
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bunnyEBO);	//��EBO
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*indices_size*sizeof(GLuint), indices, GL_STATIC_DRAW);	//�������洢��EBO����
            
        // λ������
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);	//֪ͨOPENGL��ô������Щ��������
		glEnableVertexAttribArray(0);	//������������

		/*
		glVertexAttribPointer()
		��һ������ָ����������λ�ã��붥����ɫ����layout(location=0)��Ӧ��
		�ڶ�������ָ���������Դ�С��
		����������ָ���������͡�
		���ĸ����������Ƿ�ϣ�����ݱ���׼����
		����������ǲ�����Stride����ָ���������Ķ�������֮��ļ����
		������������ʾ���ǵ�λ�������ڻ�������ʼλ�õ�ƫ����
		*/
        // ��ͨ����
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // ��ע�⣬��������ģ���glVertexAttribPointer�ĵ��ý�VBOע��Ϊ��ǰ�󶨵Ķ��㻺�����������֮�����ǿ��԰�ȫ��ȡ����
    glBindVertexArray(0); // ȡ����VAO��ȡ�����κλ�����/�����Է�ֹ������ֵĴ���ʼ����һ�����£������ס������ȡ����EBO������������VAO��
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); //�����VAO����EBO�󶨡�


    // ��Σ����õƹ��VAO����VBO��
    unsigned int lightVAO, lightVBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);	//�󶨻���
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices, GL_STATIC_DRAW);	//�ƹⶥ�����ݰ󶨵�������
    glBindVertexArray(lightVAO);	//�󶨵ƹ�VAO����
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);	//֪ͨOPENGL����������
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // ��ע�⣬��������ģ���glVertexAttribPointer�ĵ��ý�VBOע��Ϊ��ǰ�󶨵Ķ��㻺�����������֮�����ǿ��԰�ȫ��ȡ����
    glBindVertexArray(0); // ȡ����VAO��ȡ�����κλ�����/�����Է�ֹ������ֵĴ���ʼ����һ�����£������ס������ȡ����EBO������������VAO��
    

	//���ӵĳ�ʼ״̬
    modelBunny = glm::rotate(modelBunny, glm::radians(135.0f), glm::vec3(0.0f, 1.0f, 0.0f));	//��ת���� radians���û���
   
	 // ��Ⱦѭ��
    while (!glfwWindowShouldClose(window)){	//glfwWindowShouldClose ��鴰���Ƿ���Ҫ�ر�
      
	  // ÿ֡��ʱ���߼�
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // ����
        do_movement();
		
        if(!cursorDisabled) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	//�����걻���� ��ʾ���������棬��ʾ�߶Σ��������������ʾ
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	//���� ��ʾ�棬����β������ʽ

        // ��Ⱦ����
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//�����ɫ�����Լ���Ȼ���

        // ���ø�ʽ/��ͼ����ʱһ��Ҫ������ɫ��
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);

        // �ƹ�����
        glm::vec3 lightColor;	
		/*
		//������ɫ�任������glfwGetTime��ȡʱ��
         lightColor.x = sin(glfwGetTime() * 2.0f);	
         lightColor.y = sin(glfwGetTime() * 0.7f);
         lightColor.z = sin(glfwGetTime() * 1.3f);
		 */
		//��ɫ
        lightColor.x = 0.0f;
        lightColor.y = 0.0f;
        lightColor.z = 10.0f;
        glm::vec3 diffuseColor = lightColor   * glm::vec3(0.3f); // ����Ӱ��
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // ��Ӱ��

        // ���Դ
        lightingShader.setVec3("pointLights.position", PointlightPos);
        lightingShader.setVec3("pointLights.ambient", ambientColor);
        lightingShader.setVec3("pointLights.diffuse", diffuseColor);
        lightingShader.setVec3("pointLights.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights.constant", 1.0f);
        lightingShader.setFloat("pointLights.linear", (isAttenuation ? 0.014 : 0.0f));
        lightingShader.setFloat("pointLights.quadratic", (isAttenuation ? 0.0007 : 0.0f));
        
        // �۹��
        lightingShader.setVec3("spotLight.position", camera.Position);
        lightingShader.setVec3("spotLight.direction", camera.Front);
        lightingShader.setVec3("spotLight.ambient", 5.0f, 5.0f, 5.0f);  //������
        lightingShader.setVec3("spotLight.diffuse", 5.0f, 5.0f, 5.0f);  //�������
        lightingShader.setVec3("spotLight.specular", 0.0f, 0.0f, 0.0f); //����߹�
        lightingShader.setFloat("spotLight.constant", (isFlashlight? 1.0f : INF));
        lightingShader.setFloat("spotLight.linear", 0.045f);
        lightingShader.setFloat("spotLight.quadratic", 0.0075f);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(5.0f)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(9.0f)));     

        // ��������
		//lightingShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
        lightingShader.setVec3("material.ambient", 0.5f, 0.5f, 0.5f);	// ������
        lightingShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.2f);	//�������
        lightingShader.setVec3("material.specular", 1.0f, 1.0f, 1.0f); //	����ⲻ����ȫӰ�������Ĳ���(�߹�)
        lightingShader.setFloat("material.shininess", 320.0f);

        // �鿴/ͶӰת��
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();	//��ȡ�������Ұ����
        lightingShader.setMat4("projection", projection);	//��ͶӰ�����뵽��ɫ��
        lightingShader.setMat4("view", view);	//�����Ұ��������ɫ��

        // ��������ת��
        // model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        lightingShader.setMat4("model", modelBunny); //����ģ�Ͳ������뵽model��ɫ��

        // ��Ⱦ����
        glBindVertexArray(bunnyVAO);	//�����ӵ�VAO
        glDrawElements(GL_TRIANGLES, 3*indices_size, GL_UNSIGNED_INT, 0);//ͼԪ���ƺ��� 
        glBindVertexArray(0);	//ȡ����

        // �����ƵĶ���
        lampShader.use();
        lampShader.setMat4("projection", projection);
        lampShader.setMat4("view", view);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, PointlightPos);
        model = glm::scale(model, glm::vec3(0.4f)); // һ����С�������壨��Դ��
        lampShader.setMat4("model", model);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);	//���ݶ��������е��������ݺ�ָ����ģʽ�����л���
		glBindVertexArray(0);//ȡ����

        //ʰȡ�������
        if (selectedPointIndice <= vertices_size){	//����������С�ڵ�ĸ���
            selectShader.use();	//������ɫ��
			
			//����һ����ά������¼��ǰ���λ��
			//n1 n2 n3 �ֱ�Ϊ�õ��xyz����
            glm::vec4 now(modelBunny * glm::vec4(vertices[selectedPointIndice * 6], vertices[selectedPointIndice * 6 + 1], vertices[selectedPointIndice * 6 + 2], 1.0f));
            
			model = glm::translate(glm::mat4(1.0f), glm::vec3(now.x, now.y, now.z));//����һ��ƽ�ƾ��󣬵�һ������ΪĿ����󣬵ڶ���������ƽ�Ƶķ�������
            model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));	//model��һ��С�����壬scale���ڴ���һ�����ž���
            view = camera.GetViewMatrix();	//��Ұ����
            projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);	//�鿴����
           
			//��ɫ��������ؾ������
			selectShader.setMat4("projection", projection); 
            selectShader.setMat4("view", view);
            selectShader.setMat4("model", model);
            glBindVertexArray(lightVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);	//����һ��С�������ס��ѡ�ĵ㡣 ����˵�������ݶ��������е��������ݺ�ָ����ģʽ�������Σ����л���
        }
        // glfw: ��������������ѯIO�¼�������/�ͷż�������ƶ��ȣ�
        glfwSwapBuffers(window);//�����������������С������ʾ����Ļ�ϣ�;
        glfwPollEvents();
    }

    // optional: һ��ʵ����Ŀ��ȡ��������Դ����
    glDeleteVertexArrays(1, &bunnyVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &bunnyVBO);
    glDeleteBuffers(1, &lightVBO);

    // glfw: ���һ�����ͷ������ѷ����GLFW��Դ��
    glfwTerminate();
    return 0;
}

//���뺯��
void do_movement(){
    GLfloat bunnySpeed = 30.0f * deltaTime;
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
    // <- ���������ƶ�
    if (keys[GLFW_KEY_LEFT])
        modelBunny = glm::translate(modelBunny, glm::vec3(-bunnySpeed, 0, 0));
    // -> ���������ƶ�
    if (keys[GLFW_KEY_RIGHT])
        modelBunny = glm::translate(modelBunny, glm::vec3(bunnySpeed, 0, 0));
    // J ��������ת
    if (keys[GLFW_KEY_J])
        modelBunny = glm::rotate(modelBunny, glm::radians(bunnySpeed), glm::vec3(0.f, 0.f, 1.f));
    // L ��������ת
    if (keys[GLFW_KEY_L])
        modelBunny = glm::rotate(modelBunny, glm::radians(-bunnySpeed), glm::vec3(0.f, 0.f, 1.f));
    // I ������ǰת
    if (keys[GLFW_KEY_I])
        modelBunny = glm::rotate(modelBunny, glm::radians(-bunnySpeed), glm::vec3(1.f, 0.f, 0.f));
    // K �������ת
    if (keys[GLFW_KEY_K])
        modelBunny = glm::rotate(modelBunny, glm::radians(bunnySpeed), glm::vec3(1.f, 0.f, 0.f));
    // Z ���ӷŴ�
    if (keys[GLFW_KEY_Z])
        modelBunny = glm::scale(modelBunny, glm::vec3(1.0f - 0.001f * bunnySpeed, 1.0f - 0.001f * bunnySpeed, 1.0f - 0.001f * bunnySpeed));
    // X ������С
    if (keys[GLFW_KEY_X])
        modelBunny = glm::scale(modelBunny, glm::vec3(1.0f + 0.001f * bunnySpeed, 1.0f + 0.001f * bunnySpeed, 1.0f + 0.001f * bunnySpeed));
}

//�����������룺��ѯGLFW�Ƿ��ڴ�֡�а���/�ͷ�����صİ�����������Ӧ�ķ�Ӧ
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode){
    if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS){ //������¡�=����
        isAttenuation = !isAttenuation;
        std::cout << "���Դ˥���� ";
        if(isAttenuation) std::cout << "��.\n";
        else std::cout << "�ر�.\n";
        std::cout << SPLIT << std::endl;
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS){
        isFlashlight = !isFlashlight;
        std::cout << "����ƣ� ";
        if(isFlashlight) std::cout << "��.\n";
        else std::cout << "�ر�.\n";
        std::cout << SPLIT << std::endl;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){ //�������esc��رմ���
        glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS){
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED){	//��ȡ��ǰ������ģʽ���жϹ���Ƿ����
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // ������ã���ָ�ʹ�ù��
            cursorDisabled = false;	//���ñ�־
        }else{
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	//�����ǰ��겻�����ã�����ù�꣨�뿪ʰȡģʽ��
            cursorDisabled = true;
            firstMouse = true;
        }
    }
    if (key >= 0 && key < 1024){	//�ж����а����İ������ɿ�״̬
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}
// glfw: ÿ���ı䴰�ڴ�Сʱ���ͻ�ִ�лص�����
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    // ȷ���ӿ����µĴ��ڳߴ�ƥ�䣬��ע�⣬���ڵĿ�Ⱥ͸߶Ƚ������ӿ�����ʾ���ϵ�ָ���Ŀ�͸�
    glViewport(0, 0, width, height);
}


// glfw: �κ�ʱ���ƶ������øûص�����
void mouse_callback(GLFWwindow* window, double xpos, double ypos){
    mouseX = xpos;	//���x����
    mouseY = ypos;	//���y����
    if(cursorDisabled){	//���������
        if (firstMouse){	//�������һ�ӽǿ��ø��µ�ǰxy����
            lastX = xpos;	
            lastY = ypos;
            firstMouse = false;	
        }

		//����x y�����ƫ����
        float xoffset = xpos - lastX;	
        float yoffset = lastY - ypos; // ��ת����Y�������ϣ����µ���
        lastX = xpos;
        lastY = ypos;
        camera.ProcessMouseMovement(xoffset, yoffset);	//�����������ƫ�����ƶ�
    }
}

// glfw: �κ�ʱ�������ֹ���ʱ������øú�����
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    camera.ProcessMouseScroll(yoffset);
}
// ʰȡ������
void mouse_click_callback(GLFWwindow *window, int button, int action, int mods){
	//bool test = false;
    if (!cursorDisabled && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_MOUSE_BUTTON_LEFT){	//��������״̬ȡ���������ж� �������Ƿ񱻰���
	   
		//����xy����
		GLfloat xpos = mouseX;	
        GLfloat ypos = mouseY;
        std::cout << "ѡȡ�����Ļ���꣨���ڵ����Ͻ�Ϊ0,0�� : " << xpos << ' ' << ypos << std::endl;

        GLfloat minDistance = glm::pow(10, 20);	//��С����10^20
        GLuint minIndice = 0;	//��С����
        GLfloat minX =  0.0f, minY = 0.0f;
        for (int i = 0; i < vertices_size; i++){
			/*
				��modelBunny�ľ���ת��Ȼ��ת�ý������Ϊһ�����׾��� * �����ɫֵ  ��� �����ǰ���� <0 �������������ͼ��
				�ж���Щ��Ӧ�ñ�ʰȡ
			*/
			if (glm::dot(glm::mat3(glm::transpose(glm::inverse(modelBunny))) *
				glm::vec3(vertices[6 * i + 3], vertices[6 * i + 4], vertices[6 * i + 5]), camera.Front) < 0)
			{
				//std::cout << vertices[6 * i + 3] << vertices[6 * i + 4] << vertices[6 * i + 5] << std::endl;
				//std::cout << glm::vec3(vertices[6 * i + 3], vertices[6 * i + 4], vertices[6 * i + 5]).x << glm::vec3(vertices[6 * i + 3], vertices[6 * i + 4], vertices[6 * i + 5]).y<< glm::vec3(vertices[6 * i + 3], vertices[6 * i + 4], vertices[6 * i + 5]).z << std::endl;
				glm::vec4 iPos;
				glm::mat4 view = camera.GetViewMatrix();	//�������Ұ����

				glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);	//ͶӰ����,   glm::perspectives��ͶӰ����ʽ glm::radians�ǻ���

				
				iPos = modelBunny * glm::vec4(vertices[i * 6 + 0], vertices[i * 6 + 1], vertices[i * 6 + 2], 1.0f);
				iPos = projection * view * iPos;
				GLfloat pointPosX = SCR_WIDTH / 2 * (iPos.x / iPos.w) + SCR_WIDTH / 2;	//���x����
				//�ر�ע������ĸ���
				GLfloat pointPosY = SCR_HEIGHT / 2 * (-iPos.y / iPos.w) + SCR_HEIGHT / 2;	//���y����

				//�жϵ�ǰ��������Ӧ��ʰȡ�ĵ�֮��ľ��룬�����ϵĸ���minDistanceֱ�����еĵ㱻�����꣬�ó�������ĵ�����������ĵ�֮ǰ�ľ����¼ΪminDistance
				if ((pointPosX - xpos) * (pointPosX - xpos) + (pointPosY - ypos) * (pointPosY - ypos) < minDistance) {
					minDistance = (pointPosX - xpos) * (pointPosX - xpos) + (pointPosY - ypos) * (pointPosY - ypos);
					minIndice = i;
					minX = pointPosX;
					minY = pointPosY;
				}
			}
		
		}
        // ���minDistanceС��20������
        if (minDistance < 400){
            selectedPointIndice = minIndice;
            std::cout << "��������� : " << minIndice << std::endl;
            std::cout << "������� : " << vertices[minIndice * 6 + 0] << ' ' << vertices[minIndice * 6 + 1] << ' ' << vertices[minIndice * 6 + 2] << std::endl;
            std::cout << "�����Ļ���� : " << minX << ' ' << minY << std::endl;
        }else{
            std::cout << "����û�е�(����������֮��ľ������С��20������)" << std::endl;
        }
        std::cout << SPLIT << std::endl;
    }
}