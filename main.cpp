#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <random>
#include <ctime>
#include <cmath>


//this whole bitch needs a deep refactor
int randSign()
{
    return rand() % 2 == 0 ? -1 : 1;
}

void PaddleMovement(sf::RectangleShape &player, float speed)
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        player.move(sf::Vector2f(0, -speed));
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        player.move(sf::Vector2f(0, speed));
}

void PaddleCollisions(sf::RectangleShape &paddle, float paddleX)
{
    if (paddle.getPosition().y > 600 - 60) paddle.setPosition(paddleX, 600 - 60);
    if (paddle.getPosition().y < 0) paddle.setPosition(paddleX, 0);
}

void BallMovement(sf::RectangleShape &ball, float &totalSpeed, float &xSpeed, float &ySpeed, float collisionLocation, bool &spawn, 
                   bool &collision, bool &paddleCollision, bool &pointMade, sf::Sound &hit, sf::Sound &score)
{
    float ballX = ball.getPosition().x, ballY = ball.getPosition().y;
    float maxYSpeed = 7.0 / 8.0 * totalSpeed, minYSpeed = 1.0 / 3.0 * totalSpeed;
    if (spawn)
    {
        int xSign = randSign(), ySign = randSign();
        ySpeed = ySign * (fmod(rand() + maxYSpeed, maxYSpeed) + minYSpeed);
        xSpeed = xSign * sqrt(pow(totalSpeed, 2) - pow(ySpeed, 2));
        spawn = false;
    }
    
    if (paddleCollision)
    {
        float m = (2 * maxYSpeed) / 60, x = collisionLocation, b = -(maxYSpeed);
        totalSpeed += 0.3;
        hit.play();
        ySpeed = m * x + b;
        xSpeed = xSpeed < 0 ? sqrt(pow(totalSpeed, 2) - pow(ySpeed, 2)) : -(sqrt(pow(totalSpeed, 2) - pow(ySpeed, 2)));
        ballY = ballY;
        paddleCollision = false;
        if (paddleCollision) {}
    }
  
    if (collision)
    {
        hit.play();
        ySpeed = -ySpeed;
        collision = false;
    }
    
    if (pointMade)
    {
        ball.setPosition(-1000, -1000);
        score.play();
        xSpeed = 0;
        ySpeed = 0;
        pointMade = false;
    }

    ball.move(sf::Vector2f(xSpeed, ySpeed));
}

void BallSpawn(sf::RectangleShape &ball, sf::Sound &score, bool &spawn, float &totalSpeed)
{
    //check if ball is oob and if the score buffer is no longer playing, if so spawn the ball
    if ((ball.getPosition().x == -1000) && (ball.getPosition().y == -1000) && (score.getStatus() == sf::SoundSource::Status::Stopped))
    {
        ball.setPosition(400 - 5, (rand() % 300) + 150);
        spawn = true;
        totalSpeed = 4;
    }
}

void CheckBallCollisions(sf::RectangleShape &ball, sf::RectangleShape paddle1, sf::RectangleShape paddle2, 
    bool &collision, bool &paddleCollision, float &collisionLocation)
{
    float paddle1x = paddle1.getPosition().x, paddle1y = paddle1.getPosition().y, paddle2x = paddle2.getPosition().x, paddle2y = paddle2.getPosition().y;
    float ballx = ball.getPosition().x, bally = ball.getPosition().y + 5;
    
    if (((bally - 5 <= 0) || (bally >= 600 - 10)) && (ballx != -1000))
        collision = true;

    if (((ballx >= paddle1x && ballx <= paddle1x + 10) && (bally >= paddle1y && bally <= paddle1y + 60)) || //check collision for paddle 1
        ((ballx + 10 >= paddle2x && ballx <= paddle2x + 10) && (bally >= paddle2y && bally <= paddle2y + 60)))   //check collision for paddle 2
    {
        paddleCollision = true;
        ballx > 400 ? ball.setPosition(ballx - 1, bally) : ball.setPosition(ballx + 1, bally);
        collisionLocation = ballx < 400 ? bally - paddle1y : bally - paddle2y;
        // shouldnt need to check individual paddles, refactor to check for a paddle then call 2 functions to check for individual paddles
    }
}

void AIPaddle(sf::RectangleShape& paddle, sf::RectangleShape ball, float speed)
{
    float paddleY = paddle.getPosition().y, ballY = ball.getPosition().y;
    if (ball.getPosition().x == -1000)
        return;
    if (ball.getPosition().x > 395)
    {
        if (ballY > (paddleY + 30))
            paddle.move(0, speed);
        if (ballY < paddleY + 30)
            paddle.move(0, -speed);
    }
}

void ScoreTracker(sf::RectangleShape ball, bool &pointMade, int &p1score, int &p2score, sf::Text &p1Score, sf::Text &p2Score)
{
    if (ball.getPosition().x < 0 && ball.getPosition().x != -1000)
    {
        pointMade = true;
        p2score += 1;
        p2Score.setString(std::to_string(p2score));
    }

    if (ball.getPosition().x > 800)
    {
        pointMade = true;
        p1score += 1;
        p1Score.setString(std::to_string(p1score));
    }
}

bool CheckWin(int score1, int score2, int winner) 
{
    if (score1 > 10 || score2 > 10)
    {
        if (score1 > score2)
            winner = 1;
        else winner = 2;
        return true;
    }
    return false;
}

void GameOver() {}

int main()
{
    srand(time(NULL));

    // create the window
    sf::RenderWindow window(sf::VideoMode(800, 600), "Pong Clone");
    window.setFramerateLimit(120);
    
    //intitialize player variables
    float player1X = 50.0, player2X = 750, paddleSpeed = 5;
    sf::RectangleShape player1(sf::Vector2f(10, 60));
    player1.setFillColor(sf::Color(255, 255, 255));
    player1.setPosition(player1X, 300);

    //draws player 2 paddle will be refactored
    sf::RectangleShape player2(sf::Vector2f(10, 60));
    player2.setPosition(player2X, 200);
    player2.setFillColor(sf::Color(255, 255, 255));

    //draw ball
    sf::RectangleShape ball(sf::Vector2f(10, 10));
    ball.setFillColor(sf::Color(255, 255, 255));
    ball.setPosition(-1000, -1000);
    float ballSpeed = 4, ballXspeed = 0, ballYspeed = 0, collisionLocation = 0;
    bool ballSpawned = false, collision = false, paddleCollision = false;

    //draws middle division line: this is dope, no refactoring needed
    int n = 50, i = 0, j = 0;
    sf::VertexArray lines(sf::Lines, n);
    while (( i < n ) && ( j <= 600 ))
    {
        lines[i].position = sf::Vector2f(400, j);
        i++;
        j += (600 / n);
    }
    //this still dope


    //load font file
    sf::Font font;
    if (!font.loadFromFile("font/Square.ttf"))
    {
        std::cout << "Error Loading Font" << std::endl;

        return EXIT_FAILURE;
    }

    //load sound files
    sf::SoundBuffer hitBuffer;
    if (!hitBuffer.loadFromFile("sfx/Hit.wav"))
    {
        std::cout << "Error Loading ball sfx" << std::endl;

        return EXIT_FAILURE;
    }
    sf::Sound hit;
    hit.setBuffer(hitBuffer);

    sf::SoundBuffer scoreBuffer;
    if (!scoreBuffer.loadFromFile("sfx/Score.wav"))
    {
        std::cout << "Error Loading score sfx" << std::endl;

        return EXIT_FAILURE;
    }
    sf::Sound score;
    score.setBuffer(scoreBuffer);
    score.play();
    

    //draw scores
    int p1score = 0, p2score = 0, playerWinner = 0;
    sf::Text p1Score, p2Score;
    p1Score.setFont(font);
    p1Score.setCharacterSize(120);
    p1Score.setFillColor(sf::Color(255, 255, 255));
    p1Score.setPosition(275, 0);
    p1Score.setString(std::to_string(p1score));

    p2Score.setFont(font);
    p2Score.setCharacterSize(120); 
    p2Score.setFillColor(sf::Color(255, 255, 255));
    p2Score.setPosition(455, 0);
    p2Score.setString(std::to_string(p2score));

    bool pointMade = false;
    //The above code is horrendous and needs refactoring 

    // run the program as long as the window is open
    while (window.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        //paddle stuff
        PaddleMovement(player1, paddleSpeed);
        AIPaddle(player2, ball, paddleSpeed);
        PaddleCollisions(player1, player1X);
        PaddleCollisions(player2, player2X);
       
        //ball stuff
        BallSpawn(ball, score, ballSpawned, ballSpeed);
        BallMovement(ball, ballSpeed, ballXspeed, ballYspeed, collisionLocation, ballSpawned, collision, paddleCollision, pointMade, hit, score);
        CheckBallCollisions(ball, player1, player2, collision, paddleCollision, collisionLocation);

        //game stuff
        ScoreTracker(ball, pointMade, p1score, p2score, p1Score, p2Score);
        if (CheckWin(p1score, p2score, playerWinner))
        {
            GameOver();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        {
            float x = ball.getPosition().x;
            float y = ball.getPosition().y;
            int i = 0;
            continue;
        }
        
        //draws all objects
        window.clear(sf::Color::Black);
        window.draw(player1);
        window.draw(player2);
        window.draw(lines);
        window.draw(ball);
        window.draw(p1Score);
        window.draw(p2Score);
        window.display();


        // end the current frame
    }

    return 0;
}