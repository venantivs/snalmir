# snalmir
Um servidor de código aberto de With Your Destiny (WYD) para a versão 7.54 para Linux.

Este servidor é baseado no código fonte de vários outros servidores, tais como: 
- [W2PP](https://github.com/ErickAlcan/W2PP)
- [Open WYD Server](https://github.com/Rechdan/Open-WYD-Server)
- GMSRV

Este servidor está sendo desenvolvido apenas com fins educacionais.

## Roadmap
- [x] Funcionamento básico do servidor com **epoll**
- [x] Login
- [x] Login de Char (Entrar no mundo)
- [ ] Logout de Char
- [x] Criação de Char
- [x] Remoção de Char
- [ ] Mensagem de Chat
- [ ] Actions
- [ ] Mover
- [ ] ...

## TODOs gerais
- [ ] Inserção de um linter no projeto.
- [ ] Substituição de armazenamento de dados via arquivo por um SGBD.
- [ ] Substuição dos tipos nativos pelos mais intuitivos definidos em `stdint.h`.
- [ ] Adição de nome de parãmetros nas declarações de funções nos arquivos de cabeçalho.
- [ ] Renomear funções de rede que contenham `message` para `packet`.

## Setup & Execução
```bash
$ git clone git@github.com:theskytalos/snalmir.git
$ cd snalmir
$ mkdir bin
$ make
$ sudo ./bin/snalmir
```
OBS: No arquivo general-config.h há algumas configurações, tais como porta do servidor, número de canais, se o servidor http será ligado, etc.
