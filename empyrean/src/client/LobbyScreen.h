#ifndef PYR_LOBBY_SCREEN_H
#define PYR_LOBBY_SCREEN_H


#include "MenuScreen.h"
#include "MenuState.h"


namespace pyr {

    class LobbyScreen : public MenuScreen {
    public:
        LobbyScreen(MenuState* state)
            : MenuScreen(state)
        {
            phui::ButtonPtr newGame = new phui::Button("New Game");
            newGame->addListener(this, &LobbyScreen::onNewGame);
            newGame->setPositionAndSize(0, 0, 100, 100);

            phui::ButtonPtr newChar = new phui::Button("New Character");
            newChar->addListener(this, &LobbyScreen::onNewChar);
            newChar->setPositionAndSize(0, 100, 100, 100);

            phui::ButtonPtr leave = new phui::Button("Leave Server");
            leave->addListener(this, &LobbyScreen::onQuit);
            leave->setPositionAndSize(0, 200, 100, 100);

            _messages = new phui::ListBox();
            _messages->setPositionAndSize(100, 0, 924, 668);

            _text = new phui::TextField();
            _text->setPositionAndSize(100, 668, 724, 100);
        
            phui::ButtonPtr say = new phui::Button("Say");
            say->addListener(this, &LobbyScreen::onSay);
            say->setPositionAndSize(824, 668, 200, 100);
            
            add(newGame);
            add(newChar);
            add(leave);
            add(_messages);
            add(_text);
            add(say);
        }

        void update(float /*dt*/) {
            std::vector<std::string> msgs = ServerConnection::instance().getLobbyMessages();
            _messages->add(msgs);
        }
    
    private:
        void onNewGame(const phui::ActionEvent&) {
            getState()->onLobbyNewGame();
        }

        void onNewChar(const phui::ActionEvent&) {
            getState()->onLobbyNewChar();
        }

        void onQuit(const phui::ActionEvent&) {
            getState()->onLobbyQuit();
        }

        void onSay(const phui::ActionEvent&) {
            getState()->onLobbySay(_text->getText());
            _text->setText("");
        }
        
        phui::TextFieldPtr _text;
        phui::ListBoxPtr _messages;
    };

}


#endif