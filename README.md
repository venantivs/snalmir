# snalmir
Um servidor (emulador) de código aberto de With Your Destiny (WYD) para a versão 7.54 para Linux.

Toda a parte de rede é baseada em eventos assíncronos não-bloqueantes ([epoll](https://www.man7.org/linux/man-pages/man7/epoll.7.html)).

Este servidor é baseado no código fonte de vários outros servidores, tais como: 
- [W2PP](https://github.com/ErickAlcan/W2PP)
- [Open WYD Server](https://github.com/Rechdan/Open-WYD-Server)
- GMSRV

Este servidor está sendo desenvolvido apenas com fins educacionais.

<img width="802" height="627" alt="Captura de tela 2025-08-28 201113" src="https://github.com/user-attachments/assets/391c7689-632b-4a88-8d0e-f49692ebac89" />

## Limitações
Por ser um projeto para estudos, o objetivo foi simplificar ao máximo as complicações que já existem em servidores tradicionais:

- Não há suporte para multi servidores (Não há DBSrv, tudo é controlado pela "TMSrv")
- Não há suporte para multi canais
- Arquivos de "DB" usam IO bloqueante, o que pode deixar o servidor mais lento. Há planos para mudar isso.

## Roadmap
- [x] Funcionamento básico do servidor com **epoll**
- [x] Login
- [x] Login de Char (Entrar no mundo)
- [ ] Logout de Char
- [x] Criação de Char
- [x] Remoção de Char
- [ ] Mensagem de Chat
- [ ] Actions
- [x] Mover
- [ ] ...

## TODOs gerais
- [ ] Inserção de um linter no projeto.
- [ ] Substituição de armazenamento de dados via arquivo por um SGBD.
- [ ] Substuição dos tipos nativos pelos mais intuitivos definidos em `stdint.h`.
- [ ] Adição de nome de parãmetros nas declarações de funções nos arquivos de cabeçalho.
- [ ] Renomear funções de rede que contenham `message` para `packet`.
- [x] Colocar prefixo g_ em variáveis globais.

## Setup & Execução
```bash
$ git clone git@github.com:theskytalos/snalmir.git
$ cd snalmir
$ mkdir bin
$ make
$ sudo ./bin/snalmir
```
OBS: No arquivo general-config.h há algumas configurações, tais como porta do servidor, número de canais, se o servidor http será ligado, etc.
