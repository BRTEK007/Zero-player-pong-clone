#include <SFML/Graphics.hpp>
#include <stdlib.h>
#include <time.h>
#include <math.h>

constexpr int WINDOW_WIDTH = 1210;
constexpr int WINDOW_HEIGHT = 610;
constexpr int BORDER_THICKNESS = 5;
constexpr int PADDLE_OFFSET = 55;
constexpr int PADDLE_LENGTH = 100;
constexpr int PADDLE_WIDTH = 20;
constexpr int PADDLE_SPEED = 400;
constexpr int BALL_RADIUS = 15;
constexpr int BALL_SPEED = 800;
constexpr int MAX_FLIPS = 10;

const sf::Color PADDLE_COLOR = sf::Color::Yellow; 
const sf::Color BORDER_COLOR = sf::Color::White;
const sf::Color BALL_COLOR = sf::Color::Red;

void draw_path(sf::RenderWindow &window, std::vector<sf::Vector2f> &v, unsigned flips){
	sf::Vertex line[v.size()];
	for(int i = 0; i < v.size(); i++)
		line[i] = sf::Vertex(v[i], BALL_COLOR);
	
	window.draw(line, v.size(), sf::LineStrip);

	//draw checkpoints
	sf::CircleShape c;
	c.setRadius(BALL_RADIUS);
	c.setFillColor(BALL_COLOR);
	c.setOutlineThickness(1);
	c.setOutlineColor(BALL_COLOR);
	c.setOrigin(sf::Vector2f(BALL_RADIUS, BALL_RADIUS));

	//draw reached
	for(int i = 1; i < flips+1; i++){
		c.setPosition(v[i]);
		window.draw(c);
	}

	c.setFillColor(sf::Color::Black);

	//draw not reached yet
	for(int i = flips+1; i < v.size(); i++){
		c.setPosition(v[i]);
		window.draw(c);
	}
}

class Paddle{
	private:
		sf::RectangleShape s;
		float target_y;	
	public:
		sf::Vector2f pos;
		float vel;

		Paddle(int x, int y){
			pos.x = x;
			pos.y = y;
			target_y = y;
			s.setSize(sf::Vector2f(PADDLE_WIDTH, PADDLE_LENGTH));
			s.setPosition(pos);
			s.setFillColor(PADDLE_COLOR);
		}
		
		void set_target(float y){
			target_y = y;
		}

		void update(float delta){
			float h = PADDLE_LENGTH/2;
			float step = PADDLE_SPEED * delta;
			
			//if adding or substarcting step could make me closer to target_y do it
			if(abs(pos.y + h + step - target_y) < abs(pos.y + h - target_y)){
				if(pos.y + PADDLE_LENGTH + step <= WINDOW_HEIGHT - BORDER_THICKNESS)
					pos.y += step;
			}
			else if(abs(pos.y + h - step - target_y) < abs(pos.y + h - target_y)){
				if(pos.y - step >= BORDER_THICKNESS)
					pos.y -= step;
			}
			
			s.setPosition(pos);
		}

		void draw(sf::RenderWindow &w){
			w.draw(s);
		}

};

class Ball{
	private:
		sf::CircleShape s;
	
	public:
		sf::Vector2f pos;
		sf::Vector2f vel;

		Ball(int x, int y){
			pos.x = x;
			pos.y = y;

			vel.x = 0;
			vel.y = 0;

			s.setRadius(BALL_RADIUS);
			s.setPosition(pos);
			s.setFillColor(BALL_COLOR);
			s.setOrigin(sf::Vector2f(BALL_RADIUS, BALL_RADIUS));
		}

		//bounce from paddle, dir=1 -> right, -1 -> left
		void bounce(int dir){
			float k = (rand() % 24 - 12)/8.f;//y = kx + b
		       	float m = sqrt(1.f + k*k);//magnitude
			vel.x = (1.f/m) * dir * BALL_SPEED;
			vel.y = (k/m) * BALL_SPEED;
		}
		
		//flip from top or bottom of the screen
		void flip(){
			vel.y *= -1;
		}

		//translates to collision point with y
		void set_pos_by_y(float y){
			float a = vel.y / vel.x;//y = ax + b
			float b = pos.y - a * pos.x;
			pos.x = (y-b) /a;
			pos.y = y;
		}

		void update(float delta){
			pos += vel*delta;
			s.setPosition(pos);
		}

		void draw(sf::RenderWindow &w){
			w.draw(s);
		}

};

//compute path of the ball
void compute_path(const Ball &ball, const Paddle &padl, 
					const Paddle &padr, std::vector<sf::Vector2f> &out){
	int dir = ball.vel.x > 0 ? 1 : -1;
	float a = ball.vel.y / ball.vel.x;//y = ax + b
	float b = ball.pos.y - a * ball.pos.x;

	out.push_back(ball.pos);

	if(a != 0){
		for(int i = 0; i < MAX_FLIPS; i++){
			//collision with walls
			float wy = a*dir > 0 ? WINDOW_HEIGHT - BORDER_THICKNESS - BALL_RADIUS : BORDER_THICKNESS + BALL_RADIUS;
			float wx = (wy - b)/a;

			//collision with paddle
			float px = dir == 1 ? padr.pos.x - BALL_RADIUS : padl.pos.x + BALL_RADIUS + PADDLE_WIDTH;
			float py = a*px + b;

			//paddle collision, end loop
			if(dir == 1 ? px <= wx : px >= wx){
				out.push_back(sf::Vector2f(px, py));
				break;
			}

			//flip from wall, update line params
			out.push_back(sf::Vector2f(wx, wy));
			a *= -1;
			b = wy - a * wx;
		}
	}else{//going straight
		out.push_back(sf::Vector2f(dir == 1 ? padr.pos.x - BALL_RADIUS : padl.pos.x + BALL_RADIUS + PADDLE_WIDTH, ball.pos.y));
	}
}

void draw_background(sf::RenderWindow &w){
	sf::RectangleShape s;
	s.setSize(sf::Vector2f(WINDOW_WIDTH - BORDER_THICKNESS*2, WINDOW_HEIGHT - BORDER_THICKNESS*2));
	s.setPosition(BORDER_THICKNESS, BORDER_THICKNESS);
	s.setOutlineColor(BORDER_COLOR);
	s.setOutlineThickness(BORDER_THICKNESS);
	s.setFillColor(sf::Color::Black);
	w.draw(s);
}


int main(){
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), ":)");
	
	sf::Clock clock;

	srand((unsigned int)time(NULL));

	Paddle paddle_l = Paddle(BORDER_THICKNESS + PADDLE_OFFSET, WINDOW_HEIGHT/2 - PADDLE_LENGTH/2);
	Paddle paddle_r = Paddle(WINDOW_WIDTH - BORDER_THICKNESS-PADDLE_WIDTH-PADDLE_OFFSET,
		       	WINDOW_HEIGHT/2 - PADDLE_LENGTH/2);
	Ball ball = Ball(paddle_l.pos.x + PADDLE_WIDTH+BALL_RADIUS, WINDOW_HEIGHT/2-BALL_RADIUS/2);
	
	std::vector<sf::Vector2f> path;

	ball.bounce(1);//start bounce from left paddle
	compute_path(ball, paddle_l, paddle_r, path);
	paddle_r.set_target(path[path.size()-1].y);

	unsigned flips = 0;

	while(window.isOpen()){
		sf::Event e;
		while(window.pollEvent(e)){
			if(e.type == sf::Event::Closed)
				window.close();
		}

		float elapsed = clock.restart().asSeconds(); 
		
		ball.update(elapsed);
		paddle_r.update(elapsed);
		paddle_l.update(elapsed);
		
		//ball collision with top and bottom
		if(ball.pos.y - BALL_RADIUS <= BORDER_THICKNESS){
			ball.set_pos_by_y(BORDER_THICKNESS + BALL_RADIUS);
			ball.flip();
			flips++;
		}else if(ball.pos.y + BALL_RADIUS >= WINDOW_HEIGHT-BORDER_THICKNESS){
			ball.set_pos_by_y(WINDOW_HEIGHT - BORDER_THICKNESS - BALL_RADIUS);
			ball.flip();
			flips++;
		}

		//ball collision with paddles
		if(ball.vel.x > 0){
			if(ball.pos.x + BALL_RADIUS >= paddle_r.pos.x - PADDLE_WIDTH){
				ball.bounce(-1);
				flips = 0;
				path.clear();
				compute_path(ball, paddle_l, paddle_r, path);
				paddle_l.set_target(path[path.size()-1].y);
			}
		}else{
			if(ball.pos.x - BALL_RADIUS <= paddle_l.pos.x + PADDLE_WIDTH){
				ball.bounce(1);
				flips = 0;
				path.clear();
				compute_path(ball, paddle_l, paddle_r, path);
				paddle_r.set_target(path[path.size()-1].y);
			}
		}

		//draw
		window.clear();
		draw_background(window);
		draw_path(window, path, flips);
		paddle_l.draw(window);
		paddle_r.draw(window);
		ball.draw(window);
		window.display();
	}
	
	return 0;
}
