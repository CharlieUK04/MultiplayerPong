/*
 * The MIT License (MIT)
 *
 * FXGL - JavaFX Game Library
 *
 * Copyright (c) 2015-2017 AlmasB (almaslvl@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package com.almasb.fxglgames.pong;

import com.almasb.fxgl.animation.Interpolators;
import com.almasb.fxgl.app.ApplicationMode;
import com.almasb.fxgl.app.GameApplication;
import com.almasb.fxgl.app.GameSettings;
import com.almasb.fxgl.core.math.FXGLMath;
import com.almasb.fxgl.entity.Entity;
import com.almasb.fxgl.entity.SpawnData;
import com.almasb.fxgl.input.UserAction;
import com.almasb.fxgl.net.*;
import com.almasb.fxgl.physics.CollisionHandler;
import com.almasb.fxgl.physics.HitBox;
import com.almasb.fxgl.physics.PhysicsComponent;
import com.almasb.fxgl.ui.UI;
import javafx.geometry.Point2D;
import javafx.scene.input.KeyCode;
import javafx.scene.paint.Color;
import javafx.util.Duration;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.util.Arrays;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

//import java.util.UUID;

import static com.almasb.fxgl.dsl.FXGL.*;
import static com.almasb.fxglgames.pong.NetworkMessages.*;

/**
 * A simple clone of Pong.
 * Sounds from https://freesound.org/people/NoiseCollector/sounds/4391/ under CC BY 3.0.
 *
 * @author Almas Baimagambetov (AlmasB) (almaslvl@gmail.com)
 */
public class PongApp extends GameApplication implements MessageHandler<String> {

    @Override
    protected void initSettings(GameSettings settings) {
        settings.setTitle("Pong");
        settings.setVersion("1.0");
        settings.setFontUI("pong.ttf");
        settings.setApplicationMode(ApplicationMode.DEBUG);
    }

    private Entity player1;
    private Entity player2;
    //private UUID player1uuid;
    //private UUID player2uuid;
    private Connection<String> player1Con = null;
    private Connection<String> player2Con = null;
    private boolean player1Up = false;
    private boolean player1Down = false;
    private  boolean player2Up = false;
    private boolean player2Down = false;
    private Entity ball;
    private BatComponent player1Bat;
    private BatComponent player2Bat;
    private boolean resetBall = false;
    private boolean resetGame = false;
    private boolean gameWait = true;

    private Server<String> server;

    @Override
    protected void initInput() {
        //getInput().addAction(new UserAction("Up") {
        //    @Override
        //    protected void onAction() {
        //        player1Bat.up();
        //    }
//
        //    @Override
        //    protected void onActionEnd() {
        //        player1Bat.stop();
        //    }
        //}, KeyCode.W);
//
        //getInput().addAction(new UserAction("Down") {
        //    @Override
        //    protected void onAction() {
        //        player1Bat.down();
        //    }
//
        //    @Override
        //    protected void onActionEnd() {
        //        player1Bat.stop();
        //    }
        //}, KeyCode.S);
    }

    @Override
    protected void initGameVars(Map<String, Object> vars) {
        vars.put("player1score", 0);
        vars.put("player2score", 0);
    }

    @Override
    protected void initGame() {
        Writers.INSTANCE.addTCPWriter(String.class, outputStream -> new MessageWriterS(outputStream));
        Readers.INSTANCE.addTCPReader(String.class, in -> new MessageReaderS(in));

        server = getNetService().newTCPServer(55555, new ServerConfig<>(String.class));

        server.setOnConnected(connection -> {
            connection.addMessageHandlerFX(this);
        });

        getGameWorld().addEntityFactory(new PongFactory());
        getGameScene().setBackgroundColor(Color.rgb(0, 0, 5));

        initScreenBounds();
        initGameObjects();

        var t = new Thread(server.startTask()::run);
        t.setDaemon(true);
        t.start();
    }

    @Override
    protected void initPhysics() {
        getPhysicsWorld().setGravity(0, 0);

        getPhysicsWorld().addCollisionHandler(new CollisionHandler(EntityType.BALL, EntityType.WALL) {
            @Override
            protected void onHitBoxTrigger(Entity a, Entity b, HitBox boxA, HitBox boxB) {

                if(server.getConnections().size() < 2)
                    return;

                if (boxB.getName().equals("LEFT")) {
                    inc("player2score", +1);

                    server.broadcast("SCORES," + geti("player1score") + "," + geti("player2score"));

                    server.broadcast(HIT_WALL_LEFT);

                    resetBall = true;

                    if(geti("player2score") >= 10)
                    {
                        if(server.getConnections().size() >= 2) {

                            player2Con.send("GAME_WIN");
                            player1Con.send("GAME_LOSE");
                            player1Con = null;
                            player2Con = null;

                            server.broadcast("GAME_END");

                            player1Con = null;
                            player2Con = null;

                            System.out.println("Player 2 wins!");

                        }
                        resetGame = true;
                    }


                } else if (boxB.getName().equals("RIGHT")) {
                    inc("player1score", +1);

                    server.broadcast("SCORES," + geti("player1score") + "," + geti("player2score"));

                    server.broadcast(HIT_WALL_RIGHT);

                    resetBall = true;

                    if(geti("player1score") >= 10)
                    {
                        if(server.getConnections().size() >= 2) {
                            player2Con.send("GAME_LOSE");
                            player1Con.send("GAME_WIN");

                            server.broadcast("GAME_END");

                            player1Con = null;
                            player2Con = null;

                            System.out.println("Player 1 wins!");

                        }
                        resetGame = true;
                    }

                } else if (boxB.getName().equals("TOP")) {
                    server.broadcast(HIT_WALL_UP);
                } else if (boxB.getName().equals("BOT")) {
                    server.broadcast(HIT_WALL_DOWN);
                }

                getGameScene().getViewport().shakeTranslational(5);
            }
        });

        CollisionHandler ballBatHandler = new CollisionHandler(EntityType.BALL, EntityType.PLAYER_BAT) {
            @Override
            protected void onCollisionBegin(Entity a, Entity bat) {
                playHitAnimation(bat);

                server.broadcast(bat == player1 ? BALL_HIT_BAT1 : BALL_HIT_BAT2);
            }
        };

        getPhysicsWorld().addCollisionHandler(ballBatHandler);
        getPhysicsWorld().addCollisionHandler(ballBatHandler.copyFor(EntityType.BALL, EntityType.ENEMY_BAT));
    }

    @Override
    protected void initUI() {
        MainUIController controller = new MainUIController();
        UI ui = getAssetLoader().loadUI("main.fxml", controller);

        controller.getLabelScorePlayer().textProperty().bind(getip("player1score").asString());
        controller.getLabelScoreEnemy().textProperty().bind(getip("player2score").asString());

        getGameScene().addUI(ui);
    }

    protected void setOnConnected()
    {

    }


    @Override
    protected void onUpdate(double tpf) {

        //System.out.println(player1.getX() + " < X value");

        if((!player1Up && !player1Down) || (player1Up && player1Down))
        {
            player1Bat.stop();
        }
        else if(player1Up)
        {
            player1Bat.up();
        }
        else if(player1Down)
        {
            player1Bat.down();
        }

        if((!player2Up && !player2Down) || (player2Up && player2Down))
        {
            player2Bat.stop();
        }
        else if(player2Up)
        {
            player2Bat.up();
        }
        else if(player2Down)
        {
            player2Bat.down();
        }


        if(resetBall)
        {
            resetBall();
            resetBall = false;
        }
        if(resetGame)
        {
            restartGame();
            resetGame = false;
        }

        if (!server.getConnections().isEmpty()) {
            var message = "GAME_DATA," + player1.getY() + "," + player2.getY() + "," + ball.getX() + "," + ball.getY();

            server.broadcast(message);

            //if(server.getConnections().size() >= 2)
            //{
            //    player1Con = server.getConnections().get(0);
            //    player2Con = server.getConnections().get(1);
            //}

        }
    }

    private void initScreenBounds() {
        Entity walls = entityBuilder()
                .type(EntityType.WALL)
                .collidable()
                .buildScreenBounds(150);

        getGameWorld().addEntity(walls);
    }

    private void initGameObjects() {

        ball = spawn("ball", getAppWidth() / 2f - 5, getAppHeight() / 2f - 5);
        player1 = spawn("bat", new SpawnData(getAppWidth() / 5, getAppHeight() / 2f - 30).put("isPlayer", false));
        player2 = spawn("bat", new SpawnData(4 * getAppWidth() / 5f - 20, getAppHeight() / 2f - 30).put("isPlayer", false));

        player1Bat = player1.getComponent(BatComponent.class);
        player2Bat = player2.getComponent(BatComponent.class);
    }

    private void restartGame()
    {
        //ball.setAnchoredPosition(getAppWidth() / 2 - 5, getAppHeight() / 2 - 5);

        resetBall();

        player1.getComponent(PhysicsComponent.class).overwritePosition(new Point2D(getAppWidth() / 5f, getAppHeight() / 2f - 30));
        player2.getComponent(PhysicsComponent.class).overwritePosition(new Point2D(4 * getAppWidth() / 5f - 20, getAppHeight() / 2f - 30));
        //player1.setX(getAppWidth() / 4);
        //player1.setY(getAppHeight() / 2 - 30);
        //player2.setX(3 * getAppWidth() / 4 - 20);
        //player2.setY(getAppHeight() / 2 - 30);

        set("player1score", 0);
        set("player2score", 0);

    }

    private void resetBall()
    {
        ball.getComponent(PhysicsComponent.class).setLinearVelocity(5 * 60, -5 * 60);
        ball.getComponent(PhysicsComponent.class).overwritePosition(new Point2D(getAppWidth() / 2f - 5, getAppHeight() / 2f - 5));
    }

    private void playHitAnimation(Entity bat) {
        animationBuilder()
                .autoReverse(true)
                .duration(Duration.seconds(0.5))
                .interpolator(Interpolators.BOUNCE.EASE_OUT())
                .rotate(bat)
                .from(FXGLMath.random(-25, 25))
                .to(0)
                .buildAndPlay();
    }

    @Override
    public void onReceive(Connection<String> connection, String message) {


        var tokens = message.split(",");
        //System.out.println("kys" + connection);
        //System.out.println(connection);
        Arrays.stream(tokens).skip(1).forEach(key -> {

            //System.out.println(key.substring(0,4));

                    if(key.startsWith("STRT"))
                    {
                        System.out.println("Test");
                        if(key.endsWith("_UP"))
                        {
                            //System.out.println("Moving up");
                            if(connection == player1Con)
                                player1Up = true;
                            else if (connection == player2Con)
                                player2Up = true;
                        }
                        else if(key.endsWith("_DOWN"))
                        {
                            if(connection == player1Con)
                                player1Down = true;
                            else if (connection == player2Con)
                                player2Down = true;
                        }
                    }
                    else if(key.startsWith("STOP"))
                    {
                        if(key.endsWith("_UP"))
                        {
                            if(connection == player1Con)
                                player1Up = false;
                            else if (connection == player2Con)
                                player2Up = false;
                        }
                        else if(key.endsWith("_DOWN"))
                        {
                            if(connection == player1Con)
                                player1Down = false;
                            else if (connection == player2Con)
                                player2Down = false;
                        }
                    }
            else if(Objects.equals(key, "NEW_CONNECTION"))
            {
                //System.out.println("NEW CONNECTION FOUND: " + connection);
                //if(player1Con == null && player2Con == null && !server.getConnections().isEmpty())
                //{
                //    System.out.println(server.getConnections().size() + " AAAAAAAA " + server.getConnections());
                //    System.out.println(server.getConnections().get(0));
                //    player1Con = server.getConnections().get(0);
                //}
                //else if(player2Con == null && server.getConnections().size() >= 2)
                //{
                //    player2Con = server.getConnections().get(1);
                //}
                //Checks if there's 2 or more connections and sets the first 2 to player 1 and 2
                //Also checks if player 2 is different to the 2nd connection, allowing the connections to be reset it a player leaves
                if(server.getConnections().size() >= 2)
                {
                    player1Con = server.getConnections().get(0);
                    player2Con = server.getConnections().get(1);
                    System.out.println("Start Game!");
                    System.out.println("Client 1: " + player1Con + " Client 2: " + player2Con);

                    restartGame();


                }
                return;
            }


        });

        //System.out.println("connection 0: " + server.getConnections().get(0) + "  connection 1 " + server.getConnections().get(1));
        //if(server.getConnections().get(0) != player1Con || server.getConnections().get(1) != player2Con && player1Con != null && player2Con != null)
        //{
        //    server.broadcast("GAME_END");
        //    System.out.println("player 1: " + player1Con + " player 2: " + player2Con);
        //    player1Con = null;
        //    player2Con = null;
        //    System.out.println("Game ended due to client disconnection");
        //    return;
        //}

    }

    static class MessageWriterS implements TCPMessageWriter<String> {

        private OutputStream os;
        private PrintWriter out;

        MessageWriterS(OutputStream os) {
            this.os = os;
            out = new PrintWriter(os, true);
        }

        @Override
        public void write(String s) throws Exception {
            out.print(s.toCharArray());
            out.flush();
        }
    }

    static class MessageReaderS implements TCPMessageReader<String> {

        private BlockingQueue<String> messages = new ArrayBlockingQueue<>(50);

        private InputStreamReader in;

        MessageReaderS(InputStream is) {
            in =  new InputStreamReader(is);

            var t = new Thread(() -> {
                try {

                    char[] buf = new char[36];

                    int len;

                    while ((len = in.read(buf)) > 0) {
                        var message = new String(Arrays.copyOf(buf, len));

                        System.out.println("Recv message: " + message);

                        messages.put(message);
                    }

                } catch (Exception e) {
                    e.printStackTrace();
                }
            });

            t.setDaemon(true);
            t.start();
        }

        @Override
        public String read() throws Exception {
            return messages.take();
        }
    }

    public static void main(String[] args) {
        launch(args);
    }
}
