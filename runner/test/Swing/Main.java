import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class Main {
    public static void main(String[] args) {
        JFrame frame = new JFrame("Simple Swing Demo");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(300, 200);
        
        frame.setLayout(new FlowLayout());

        JLabel label = new JLabel("this is a label");
        JButton button = new JButton("*button*");

        button.addActionListener(new ActionListener() {
            int count = 0;
            @Override
            public void actionPerformed(ActionEvent e) {
                count++;
                label.setText("You have clicked the button: " + count + " times");
            }
        });

        frame.add(label);
        frame.add(button);

        frame.setLocationRelativeTo(null);
        frame.setVisible(true);
    }
}