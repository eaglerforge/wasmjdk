import org.lwjgl.glfw.*;
import org.lwjgl.opengles.*;
import org.lwjgl.system.*;

import java.nio.*;

import static org.lwjgl.glfw.GLFW.*;
import static org.lwjgl.opengles.GLES20.*;
import static org.lwjgl.system.MemoryUtil.*;

public class Main {

    public static void main(String[] args) {
        System.out.println("LWJGL Graphics Test");
        System.out.flush();
        if (!glfwInit()) {
            System.out.println("Failed to init GLFW.");
            System.out.flush();
            throw new IllegalStateException("Unable to initialize GLFW");
        }
        System.out.println("Prepping window hints...");
        System.out.flush();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        System.out.println("Creating window...");
        System.out.flush();

        long window = glfwCreateWindow(300, 200, "JVM Wasm Test", NULL, NULL);
        if (window == NULL) {
            System.out.println("Failed to create window");
            System.out.flush();
            throw new RuntimeException("Failed to create the GLFW window");
        }
        System.out.println("Window created...");
        System.out.flush();

        glfwMakeContextCurrent(window);

        GLES.createCapabilities();

        System.out.println("Drawing...");
        System.out.flush();

        while (!glfwWindowShouldClose(window)) {
            glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glfwDestroyWindow(window);
        glfwTerminate();
        System.out.println("Terminating...");
        System.out.flush();
    }
}