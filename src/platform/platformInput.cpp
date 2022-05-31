#include "platformInput.h"

platform::Button keyBoard[platform::Button::BUTTONS_COUNT];
platform::Button leftMouse;
platform::Button rightMouse;

platform::ControllerButtons controllerButtons;
std::string typedInput;

int platform::isKeyHeld(int key)
{
	if (key < Button::A || key >= Button::BUTTONS_COUNT) { return 0; }

	return keyBoard[key].held;
}

int platform::isKeyPressedOn(int key)
{
	if (key < Button::A || key >= Button::BUTTONS_COUNT) { return 0; }

	return keyBoard[key].pressed;
}

int platform::isKeyReleased(int key)
{
	if (key < Button::A || key >= Button::BUTTONS_COUNT) { return 0; }

	return keyBoard[key].released;
}

int platform::isKeyTyped(int key)
{
	if (key < Button::A || key >= Button::BUTTONS_COUNT) { return 0; }

	return keyBoard[key].typed;
}

int platform::isLMousePressed()
{
	return leftMouse.pressed;
}

int platform::isRMousePressed()
{
	return rightMouse.pressed;
}

int platform::isLMouseReleased()
{
	return leftMouse.released;
}

int platform::isRMouseReleased()
{
	return rightMouse.released;
}


int platform::isLMouseHeld()
{
	return leftMouse.held;
}

int platform::isRMouseHeld()
{
	return rightMouse.held;
}

platform::ControllerButtons platform::getControllerButtons()
{
	return platform::isFocused() ? controllerButtons : platform::ControllerButtons{};
}

std::string platform::getTypedInput()
{
	return typedInput;
}

void platform::internal::setButtonState(int button, int newState)
{

	processEventButton(keyBoard[button], newState);

}

void platform::internal::setLeftMouseState(int newState)
{
	processEventButton(leftMouse, newState);

}

void platform::internal::setRightMouseState(int newState)
{
	processEventButton(rightMouse, newState);

}


void platform::internal::updateAllButtons(float deltaTime)
{
	for (int i = 0; i < platform::Button::BUTTONS_COUNT; i++)
	{
		updateButton(keyBoard[i], deltaTime);
	}

	updateButton(leftMouse, deltaTime);
	updateButton(rightMouse, deltaTime);
	
	for(int i=0; i<=GLFW_JOYSTICK_LAST; i++)
	{
		if(glfwJoystickPresent(i) && glfwJoystickIsGamepad(i))
		{
			GLFWgamepadstate state;

			if (glfwGetGamepadState(i, &state))
			{
				for (int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; i++)
				{
					if(state.buttons[i] == GLFW_PRESS)
					{
						processEventButton(controllerButtons.buttons[i], 1);
					}else
					if (state.buttons[i] == GLFW_RELEASE)
					{
						processEventButton(controllerButtons.buttons[i], 0);
					}
					updateButton(controllerButtons.buttons[i], deltaTime);

				}
				
				controllerButtons.LT = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER];
				controllerButtons.RT = state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];

				controllerButtons.LStick.x = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
				controllerButtons.LStick.y = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];

				controllerButtons.RStick.x = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
				controllerButtons.RStick.y = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
			
				break;
			}

		}

	}


}

void platform::internal::resetInputsToZero()
{
	resetTypedInput();

	for (int i = 0; i < platform::Button::BUTTONS_COUNT; i++)
	{
		resetButtonToZero(keyBoard[i]);
	}

	resetButtonToZero(leftMouse);
	resetButtonToZero(rightMouse);
	
	controllerButtons.setAllToZero();
}

void platform::internal::addToTypedInput(char c)
{
	typedInput += c;
}

void platform::internal::resetTypedInput()
{
	typedInput.clear();
}
