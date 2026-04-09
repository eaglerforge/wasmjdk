import org.lwjgl.glfw.*;
import org.lwjgl.opengl.*;
import org.lwjgl.system.*;

import static org.lwjgl.glfw.Callbacks.*;
import static org.lwjgl.glfw.GLFW.*;
import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.system.MemoryUtil.*;

import org.lwjgl.system.Configuration;
import java.lang.Math;

public class Main {
    static long window = 0;
    static float timer = 0.0f;

    public static void frame() {
        timer += 0.02f;
        float bgR = (float)((Math.sin(timer) * 0.5f) + 0.5f);
        glClearColor(bgR, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        glDisable(GL_ALPHA_TEST);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(timer * 50f, 0, 0, 1);
        glBegin(GL_TRIANGLES);
            glColor3f(1, 0, 0); glVertex3f(-0.5f, -0.5f, 0f);
            glColor3f(0, 1, 0); glVertex3f(0.5f, -0.5f, 0f);
            glColor3f(0, 0, 1); glVertex3f(0f, 0.5f, 0f);
        glEnd();
        glFlush();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    public static void main(String[] args) {
        //Configuration.DEBUG.set(true);
        //Configuration.DEBUG_FUNCTIONS.set(true);

        if (!glfwInit()) {
            throw new IllegalStateException("Unable to initialize GLFW");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); 
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(300, 300, "Blue Window", NULL, NULL);
        if (window == NULL) {
            throw new RuntimeException("Failed to create the GLFW window");
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        glfwShowWindow(window);

        GL.createCapabilities();

        while (!glfwWindowShouldClose(window)) {
            Main.frame();
            try {
                Thread.sleep(16);
            } catch (InterruptedException e) {

            }
        }

        glfwFreeCallbacks(window);
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}