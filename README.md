# snalmir
Um servidor de código aberto de With Your Destiny (WYD) para a versão 7.54

## Roadmap
- ~~Funcionamento básico do servidor com **epoll**~~
- Login
- Login de Char
- Logout de Char
- Criação de Char
- Remoção de Char
- Mensagem de Chat
- Actions
- Mover
- ...

## Setup & Execução
```bash
$ git clone git@github.com:theskytalos/snalmir.git
$ cd snalmir
$ mkdir bin
$ make
$ sudo ./bin/snalmir
```
OBS: No arquivo general-config.h há algumas configurações, tais como porta do servidor, número de canais, se o servidor http será ligado, etc.
