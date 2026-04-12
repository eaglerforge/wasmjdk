import java.lang.reflect.Field;
import java.lang.reflect.Method;

public class Main {
    public static void main(String[] args) {
        try {
            TestObject obj = new TestObject("Initial Secret");

            Field privateField = TestObject.class.getDeclaredField("secretMessage");
            privateField.setAccessible(true);
            
            System.out.println("original field val: " + privateField.get(obj));
            
            privateField.set(obj, "overridden through reflection");
            System.out.println("new field val: " + obj.getSecretMessage());

            Method privateMethod = TestObject.class.getDeclaredMethod("hiddenGreeting", String.class);
            privateMethod.setAccessible(true);
            
            String result = (String) privateMethod.invoke(obj, "ifyouseethisitworks");
            System.out.println("Method.invoke(): " + result);

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}

class TestObject {
    private String secretMessage;

    public TestObject(String secretMessage) {
        this.secretMessage = secretMessage;
    }

    public String getSecretMessage() {
        return secretMessage;
    }

    private String hiddenGreeting(String name) {
        return "arg:" + name + " from a private method";
    }
}