#pragma once
#include <GLFW/glfw3.h>
#include "gameLayer.h"
#include <string>

namespace platform
{
	struct Button
	{
		char pressed = 0;
		char held = 0;
		char released = 0;
		char newState = -1; // this can be -1, used for internal logic
		char typed = 0;
		float typedTime = 0;


		enum
		{
			A = 0,
			B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
			NR0, NR1, NR2, NR3, NR4, NR5, NR6, NR7, NR8, NR9,
			Space,
			Enter,
			Escape,
			Up,
			Down,
			Left,
			Right,
			LeftCtrl,
			Tab,
			BUTTONS_COUNT, //
		};

		//static constexpr int buttonValues[BUTTONS_COUNT] =
		//{
		//	GLFW_KEY_A, GLFW_KEY_B, GLFW_KEY_C, GLFW_KEY_D, GLFW_KEY_E, GLFW_KEY_F, GLFW_KEY_G,
		//	GLFW_KEY_H, GLFW_KEY_I, GLFW_KEY_J, GLFW_KEY_K, GLFW_KEY_L, GLFW_KEY_M, GLFW_KEY_N,
		//	GLFW_KEY_O, GLFW_KEY_P, GLFW_KEY_Q, GLFW_KEY_R, GLFW_KEY_S, GLFW_KEY_T, GLFW_KEY_U, 
		//	GLFW_KEY_V, GLFW_KEY_W, GLFW_KEY_X, GLFW_KEY_Y, GLFW_KEY_Z,
		//	GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6,
		//	GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9,
		//	GLFW_KEY_SPACE, GLFW_KEY_ENTER, GLFW_KEY_ESCAPE, GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT
		//};

		void merge(const Button &b)
		{
			this->pressed |= b.pressed;
			this->released |= b.released;
			this->held |= b.held;
		}
	};

	namespace internal
	{
		inline void resetButtonToZero(Button &b)
		{
			b.pressed = 0;
			b.held = 0;
			b.released = 0;
		}
	}

	struct ControllerButtons
	{
		enum Buttons
		{
			A = GLFW_GAMEPAD_BUTTON_A,           
			B = GLFW_GAMEPAD_BUTTON_B,           
			X = GLFW_GAMEPAD_BUTTON_X,           
			Y = GLFW_GAMEPAD_BUTTON_Y,           
			LBumper = GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, 
			RBumper = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
			Back = GLFW_GAMEPAD_BUTTON_BACK,
			Start = GLFW_GAMEPAD_BUTTON_START,       
			Guide = GLFW_GAMEPAD_BUTTON_GUIDE,      
			LThumb = GLFW_GAMEPAD_BUTTON_LEFT_THUMB,  
			Rthumb = GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, 
			Up = GLFW_GAMEPAD_BUTTON_DPAD_UP,   
			Right = GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,  
			Down = GLFW_GAMEPAD_BUTTON_DPAD_DOWN, 
			Left = GLFW_GAMEPAD_BUTTON_DPAD_LEFT,  
		};

		Button buttons[GLFW_GAMEPAD_BUTTON_LAST + 1] = {};

		float LT = 0.f;
		float RT = 0.f;

		struct
		{
			float x = 0.f, y = 0.f;
		}LStick, RStick;

		void setAllToZero()
		{
			*this = ControllerButtons();
		}
	};

	
	//Button::key
	int isKeyHeld(int key);
	int isKeyPressedOn(int key);
	int isKeyReleased(int key);
	int isKeyTyped(int key);

	int isLMousePressed();
	int isRMousePressed();

	int isLMouseReleased();
	int isRMouseReleased();

	int isLMouseHeld();
	int isRMouseHeld();

	ControllerButtons getControllerButtons();
	std::string getTypedInput();

	namespace internal
	{

		void setButtonState(int button, int newState);

		void setLeftMouseState(int newState);
		void setRightMouseState(int newState);

		inline void processEventButton(Button &b, bool newState)
		{
			b.newState = newState;
		}

		inline void updateButton(Button &b, float deltaTime)
		{
			if (b.newState == 1)
			{
				if (b.held)
				{
					b.pressed = false;
				}
				else
				{
					b.pressed = true;
				}

				b.held = true;
				b.released = false;
			}
			else if(b.newState == 0)
			{
				b.held = false;
				b.pressed = false;
				b.released = true;
			}else
			{
				b.pressed = false;
				b.released = false;
			}

			//processing typed
			if (b.pressed)
			{
				b.typed = true;
				b.typedTime = 0.48f;
			}
			else if(b.held)
			{
				b.typedTime -= deltaTime;
			
				if (b.typedTime < 0.f)
				{
					b.typedTime += 0.07f;
					b.typed = true;
				}
				else
				{
					b.typed = false;
				}

			}
			else
			{
				b.typedTime = 0;
				b.typed = false;
			}


			b.newState = -1;
		}


		void updateAllButtons(float deltaTime);
		void resetInputsToZero();

		void addToTypedInput(char c);
		void resetTypedInput();

	};

};