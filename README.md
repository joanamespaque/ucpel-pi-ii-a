# Monitor de Rede em C

Projeto Integrador II-A: **Monitor de Rede com Linguagem C** (UCPel)

Aplicação de linha de comando escrita em C (padrão C99) que coleta e exibe informações básicas da rede:
- endereços IP das interfaces;
- tipo de IP (IPv4 ou IPv6);
- latência;
- taxa de pacotes perdidos.

O mesmo código-fonte compila e executa em **Linux** e em **Windows**.

---

## Sumário

1. [Sobre o Projeto Integrador](#1-sobre-o-projeto-integrador)
2. [Fundamentação teórica](#2-fundamentação-teórica)
3. [Funcionalidades](#3-funcionalidades)
4. [Arquitetura do código](#4-arquitetura-do-código)
5. [Relação com o Pensamento Computacional](#5-relação-com-o-pensamento-computacional)
6. [Compilação e execução](#6-compilação-e-execução)
7. [Exemplo de uso](#7-exemplo-de-uso)
8. [Decisões técnicas e limitações](#8-decisões-técnicas-e-limitações)
9. [Referências](#9-referências)

---

## 1. Sobre o Projeto Integrador

### 1.1 Tema e modelo

| Item | Descrição |
|------|-----------|
| **Tema** | Monitor de Rede com Linguagem C |
| **Modelo proposto** | Teórico-prático |
| **Disciplinas integradas** | Pensamento Computacional e Fundamentos de Redes de Computadores |

### 1.2 Objetivo

O projeto integra os conhecimentos das disciplinas no desenvolvimento de um **monitor de rede simples**. Para isso, usa algoritmos sequenciais escritos em uma linguagem de programação e aplica o que foi estudado sobre protocolos de rede.

A aplicação poderá ser incorporada ao **HUSFP** (Hospital Universitário São Francisco de Paula), como parte da prática extensionista prevista na proposta do projeto integrador. Em um ambiente hospitalar, a disponibilidade da rede é crítica. Uma ferramenta leve, que mostre de forma direta os endereços da máquina, a latência até um serviço e a perda de pacotes, ajuda no diagnóstico inicial de problemas de conectividade.

### 1.3 Etapas do projeto

1. **Pesquisa:** sistematizar os conceitos de *Pensamento Computacional* e *Fundamentos de Redes de Computadores*, contextualizados nas atividades do Projeto Integrador II-A.
2. **Desenvolvimento:** implementar em C uma rotina que funcione como monitor de rede, coletando e exibindo:
   - endereços IP;
   - tipo de IP (IPv4 ou IPv6);
   - latência;
   - taxa de pacotes perdidos.
3. **Entrega parcial (40% da nota):** relatório conceitual sobre:
   - o histórico da linguagem C e seu perfil de aplicação;
   - as características, cabeçalhos, classes e a estrutura de endereçamento do IPv4 e do IPv6;
   - os comandos de coleta de informações de rede em diferentes sistemas operacionais.
4. **Entrega final (60% da nota):** relatório com o ambiente de desenvolvimento, os procedimentos realizados, capturas de tela e os códigos da aplicação, além de um vídeo de apresentação de até 10 minutos.

### 1.4 Critérios de avaliação

**Entrega parcial**

| Critério | Peso |
|----------|------|
| Histórico da linguagem C e seu perfil de aplicação | 3,0 |
| Características dos protocolos IPv4 e IPv6 | 3,0 |
| Exemplos de comandos para coleta de informações de rede | 2,0 |
| Qualidade do relatório parcial | 2,0 |

**Entrega final**

| Critério | Peso |
|----------|------|
| Qualidade e funcionalidade da implementação | 3,0 |
| Grau de entendimento dos códigos desenvolvidos | 2,5 |
| Qualidade da apresentação do projeto | 2,5 |
| Qualidade do relatório final desenvolvido | 2,0 |

---

## 2. Fundamentação teórica

### 2.1 A linguagem C

A linguagem C foi criada por **Dennis Ritchie** nos **Laboratórios Bell** (AT&T), entre 1969 e 1973. Ela evoluiu das linguagens BCPL e B e nasceu para reescrever o sistema operacional **UNIX**, que antes era escrito em Assembly. Com isso, o UNIX tornou-se portável entre arquiteturas diferentes.

Marcos importantes:
- **1978:** publicação do livro *The C Programming Language* (Kernighan & Ritchie), conhecido como "K&R C";
- **1989/1990:** padronização ANSI/ISO (C89/C90);
- **revisões posteriores:** C99, C11, C17 e C23.

Seu perfil de aplicação é a **programação de sistemas**:
- sistemas operacionais (Linux e núcleos do Windows);
- drivers;
- sistemas embarcados e firmware;
- compiladores;
- bancos de dados;
- **pilhas de protocolos de rede**.

As características que tornam o C adequado a esse perfil são:
- acesso direto à memória por meio de ponteiros;
- baixo *overhead*;
- alta portabilidade;
- proximidade com o hardware.

As APIs de *sockets* (Berkeley Sockets e Winsock), usadas neste projeto, são interfaces em C.

### 2.2 Endereçamento IP

O **IP (Internet Protocol)** é o protocolo da camada de rede responsável por endereçar e encaminhar datagramas entre redes.

#### IPv4

- Endereço de **32 bits**, escrito em notação decimal pontuada (ex.: `192.168.0.10`).
- O cabeçalho tem de **20 a 60 bytes**. Seus campos são:
  - Versão;
  - IHL;
  - Tipo de Serviço/DSCP;
  - Comprimento Total;
  - Identificação;
  - Flags;
  - Deslocamento do Fragmento;
  - **TTL**;
  - Protocolo;
  - Checksum do cabeçalho;
  - Endereço de Origem e de Destino;
  - Opções.
- **Endereçamento classful (histórico):**

  | Classe | Faixa do 1º octeto | Uso |
  |--------|--------------------|-----|
  | A | 0 – 127 | Grandes redes (máscara padrão /8) |
  | B | 128 – 191 | Redes médias (/16) |
  | C | 192 – 223 | Redes pequenas (/24) |
  | D | 224 – 239 | Multicast |
  | E | 240 – 255 | Reservada/experimental |

- **Faixas especiais:**
  - privadas (RFC 1918): `10.0.0.0/8`, `172.16.0.0/12` e `192.168.0.0/16`;
  - *loopback*: `127.0.0.0/8`;
  - link-local: `169.254.0.0/16`.
- Hoje usa-se o **CIDR** (ex.: `/24`) no lugar das classes.

#### IPv6

- Endereço de **128 bits**, escrito em oito grupos hexadecimais de 16 bits (ex.: `2001:db8::1`). Zeros consecutivos podem ser abreviados com `::`.
- Tem um **cabeçalho fixo de 40 bytes**, mais simples que o do IPv4. Seus campos são:
  - Versão;
  - Classe de Tráfego;
  - Rótulo de Fluxo;
  - Comprimento da Carga;
  - Próximo Cabeçalho;
  - **Limite de Saltos** (equivalente ao TTL);
  - Origem e Destino.

  Opções vão em *cabeçalhos de extensão*. O IPv6 não tem checksum de cabeçalho, e a fragmentação é feita apenas na origem.
- **Tipos de endereço:**
  - *unicast* global (`2000::/3`);
  - *unique local* (`fc00::/7`);
  - *link-local* (`fe80::/10`);
  - *loopback* (`::1`);
  - *multicast* (`ff00::/8`).

  O IPv6 não tem *broadcast*.
- Resolve o esgotamento de endereços do IPv4 e oferece autoconfiguração (SLAAC).

### 2.3 ICMP, latência e perda de pacotes

- O **ICMP** (*Internet Control Message Protocol*) transporta mensagens de controle e erro do IP. A ferramenta `ping` envia mensagens **Echo Request** (tipo 8 no ICMPv4, 128 no ICMPv6) e aguarda **Echo Reply** (tipo 0 no ICMPv4, 129 no ICMPv6).
- **Latência (RTT, *round-trip time*):** tempo entre o envio do *Echo Request* e o recebimento do *Echo Reply*, medido em milissegundos.
- **Taxa de perda de pacotes:**

  ```
  perda (%) = (pacotes enviados − pacotes recebidos) × 100 / pacotes enviados
  ```

  Um pacote é considerado perdido quando a resposta não chega dentro do tempo limite (*timeout*).

### 2.4 Comandos de rede em diferentes sistemas operacionais

| Finalidade | Linux | Windows | macOS |
|------------|-------|---------|-------|
| Listar endereços IP | `ip addr` / `ifconfig` / `hostname -I` | `ipconfig` / `ipconfig /all` | `ifconfig` |
| Testar conectividade e latência | `ping -c 4 host` / `ping -6 host` | `ping -n 4 host` / `ping -6 host` | `ping -c 4 host` / `ping6 host` |
| Rota até o destino | `traceroute host` / `tracepath host` | `tracert host` | `traceroute host` |
| Tabela de rotas | `ip route` / `route -n` | `route print` | `netstat -rn` |
| Conexões e portas | `ss -tulpn` / `netstat -tulpn` | `netstat -ano` | `netstat -an` |
| Consulta DNS | `dig host` / `nslookup host` | `nslookup host` | `dig host` |
| Tabela ARP / vizinhos | `ip neigh` / `arp -n` | `arp -a` | `arp -a` |
| Latência + perda por salto | `mtr host` | `pathping host` | `mtr host` |

O programa usa as **mesmas APIs do sistema** que esses comandos usam internamente:
- no Linux, `getifaddrs()`, que também é usada pelo `ip addr`, e sockets ICMP, usados pelo `ping`;
- no Windows, `GetAdaptersAddresses()`, usada pelo `ipconfig`, e `IcmpSendEcho()`, usada pelo `ping`.

---

## 3. Funcionalidades

- Exibe o cabeçalho **"Monitor de Rede"** e o sistema operacional detectado.
- **Lista os endereços IP** de todas as interfaces ativas. Para cada endereço, mostra:
  - o nome da interface;
  - o **tipo de IP (IPv4/IPv6)**;
  - o endereço com o prefixo CIDR;
  - o **escopo** (Loopback, Private, Link-local, Public/Global, Unique local, Multicast);
  - a **classe** (A–E) para endereços IPv4.

  Ao final, mostra um resumo com a quantidade de endereços IPv4 e IPv6.
- **Mede a latência** até qualquer host, informado por nome ou por endereço IPv4/IPv6, enviando de 1 a 100 pacotes ICMP Echo. Mostra o tempo de cada resposta e o **mínimo, a média e o máximo** do RTT, com uma classificação qualitativa (Excellent, Good, Fair ou Poor).
- **Calcula a taxa de pacotes perdidos**, também com classificação (No loss, Acceptable, Unstable connection ou Host unreachable).
- Oferece um **relatório completo**, que mostra os endereços e a latência até `8.8.8.8`.
- **Valida as entradas** do usuário: rejeita opções inválidas, texto no lugar de números e hosts que não podem ser resolvidos.

---

## 4. Arquitetura do código

O código está dividido em **camadas**. Cada módulo tem uma única responsabilidade, e as diferenças entre Linux e Windows ficam isoladas com `#ifdef _WIN32`.

```
.
├── include/
│   ├── platform.h     # Cabeçalhos de socket do SO, init/cleanup, sleep e relógio
│   ├── ip_utils.h     # Conversão e classificação de endereços IP
│   ├── interfaces.h   # Estrutura ip_entry e coleta das interfaces
│   ├── ping.h         # Estrutura ping_stats e medição via ICMP
│   ├── report.h       # Formatação da saída
│   └── menu.h         # Menu interativo
├── src/
│   ├── platform.c
│   ├── ip_utils.c
│   ├── interfaces.c
│   ├── ping.c
│   ├── report.c
│   ├── menu.c
│   └── main.c
├── Makefile           # Build com GCC (Linux) ou MinGW (Windows)
└── CMakeLists.txt     # Build alternativo (Visual Studio, CLion, MinGW)
```

| Camada | Responsabilidade | Linux | Windows |
|--------|------------------|-------|---------|
| `platform` | Inicialização da rede, pausa entre pacotes, relógio de alta precisão | `clock_gettime(CLOCK_MONOTONIC)`, `nanosleep` | `WSAStartup`, `QueryPerformanceCounter`, `Sleep` |
| `ip_utils` | `inet_ntop`, tipo IPv4/IPv6, escopo e classe | comum | comum |
| `interfaces` | Coleta dos endereços de cada interface ativa | `getifaddrs()` | `GetAdaptersAddresses()` (IP Helper) |
| `ping` | Resolução DNS (`getaddrinfo`), envio de ICMP Echo, RTT e perda | socket ICMP *datagram* → socket *raw* → comando `ping` | `IcmpSendEcho` / `Icmp6SendEcho2` |
| `report` | Tabelas, estatísticas e classificações | comum | comum |
| `menu` / `main` | Laço interativo com `while`, `switch` e `scanf` | comum | comum |

### Fluxo de execução

```
main()
 ├─ platform_init()             inicializa a rede (Winsock no Windows)
 ├─ print_header()              "Monitor de Rede"
 ├─ run_menu()                  while (option != 0)
 │   ├─ 1 → collect_interfaces() → print_interfaces()
 │   ├─ 2 → ping_host()          → print_ping_stats()
 │   └─ 3 → opções 1 e 2 em sequência
 └─ platform_cleanup()
```

### Como a latência é medida no Linux

1. Primeiro, o programa tenta abrir um **socket ICMP do tipo datagram** (`SOCK_DGRAM` + `IPPROTO_ICMP`). Esse tipo não exige privilégios quando o grupo do usuário está em `net.ipv4.ping_group_range`, o que é o padrão em várias distribuições.
2. Se não conseguir, tenta um **socket raw** (`SOCK_RAW`), que exige `root` ou `CAP_NET_RAW`. Nesse caso, o próprio programa calcula o *checksum* ICMP e filtra as respostas pelo identificador.
3. Se nenhum socket puder ser aberto, executa o **comando `ping` do sistema** com o endereço já resolvido, para evitar *shell injection*, e extrai o tempo do campo `time=`.

O tempo de cada pacote é medido com um relógio monotônico, com resolução de microssegundos.

---

## 5. Relação com o Pensamento Computacional

| Conceito | Onde aparece |
|----------|--------------|
| **Decomposição** | O problema foi dividido em módulos independentes (plataforma, IP, interfaces, ping, relatório, menu). |
| **Abstração** | `platform.h` esconde as diferenças entre Linux e Windows. As structs `ip_entry` e `ping_stats` representam os dados coletados. |
| **Reconhecimento de padrões** | A mesma função `run_probes()` executa qualquer método de sondagem, recebido como ponteiro para função. |
| **Algoritmos** | O cálculo de mínimo, média, máximo e perda (`compute_summary`), o checksum ICMP e a contagem de bits da máscara (prefixo CIDR). |
| `for` | Percorre interfaces, pacotes enviados e bytes da máscara de rede. |
| `if` / `else` | Classifica endereços (tipo, escopo, classe), latência e perda. |
| `while` / `do-while` | Laço do menu, validação de entrada, espera por respostas ICMP e *retry* de `GetAdaptersAddresses`. |
| `switch` | Seleção da opção do menu. |
| `scanf` / `printf` | Leitura das opções, do host e da quantidade de pacotes, e exibição dos resultados. |

---

## 6. Compilação e execução

### Linux

Requisitos: `gcc` e `make`. Opcionalmente, `cmake`.

```bash
make                      # gera bin/network_monitor
./bin/network_monitor
```

Ou com CMake:

```bash
cmake -S . -B build
cmake --build build
./build/network_monitor
```

> **Permissões de ICMP:** se a mensagem *"ICMP is not available"* aparecer, use uma destas opções:
> - execute com `sudo`;
> - libere o ICMP sem privilégios: `sudo sysctl -w net.ipv4.ping_group_range="0 2147483647"`;
> - instale o comando `ping` (pacote `iputils-ping`), que é usado como alternativa.

### Windows

**MinGW-w64 / MSYS2:**

```bash
mingw32-make              # ou: make
bin\network_monitor.exe
```

**Visual Studio / CMake:**

```bat
cmake -S . -B build
cmake --build build --config Release
build\Release\network_monitor.exe
```

No Windows, a API `IcmpSendEcho` **não exige privilégios de administrador**. As bibliotecas `iphlpapi` e `ws2_32` são ligadas automaticamente.

### Compilação cruzada (Linux → Windows)

```bash
make CC=x86_64-w64-mingw32-gcc    # gera bin/network_monitor.exe
```

### Limpeza

```bash
make clean
```

---

## 7. Exemplo de uso

Saída ilustrativa em uma máquina Linux com IPv4 e IPv6 (os endereços variam conforme a rede):

```
==============================================================================
                               Monitor de Rede
==============================================================================
Platform: Linux

1 - List IP addresses and IP type
2 - Measure latency and packet loss
3 - Full report (addresses + latency to 8.8.8.8)
0 - Exit
Choose an option: 1

[IP addresses]
------------------------------------------------------------------------------
Interface          Type  Address/Prefix                           Scope         Class
------------------------------------------------------------------------------
lo                 IPv4  127.0.0.1/8                              Loopback      A
eth0               IPv4  192.168.0.15/24                          Private       C
lo                 IPv6  ::1/128                                  Loopback      -
eth0               IPv6  fe80::a00:27ff:fe4e:66a1/64              Link-local    -
------------------------------------------------------------------------------
Total: 4 address(es) | IPv4: 2 | IPv6: 2

Choose an option: 2
Host or IP address (e.g. 8.8.8.8, google.com, ::1): 127.0.0.1
Number of packets (1-100): 3

Sending 3 ICMP echo request(s) to 127.0.0.1...

[Latency and packet loss]
------------------------------------------------------------------------------
Target : 127.0.0.1 (127.0.0.1, IPv4)
Method : ICMP datagram socket
------------------------------------------------------------------------------
Reply from 127.0.0.1: seq=1 time=0.040 ms
Reply from 127.0.0.1: seq=2 time=0.036 ms
Reply from 127.0.0.1: seq=3 time=0.043 ms
------------------------------------------------------------------------------
Packets: sent=3 received=3 lost=0
Packet loss rate: 0.0% (No loss)
Latency: min=0.036 ms avg=0.040 ms max=0.043 ms (Excellent)
```

Quando o destino não responde, cada pacote aparece como `Request seq=N timed out`, e a taxa de perda indica `100.0% (Host unreachable)`.

A entrada também pode ser enviada por *pipe* para uma execução não interativa:

```bash
printf "3\n0\n" | ./bin/network_monitor
```

---

## 8. Decisões técnicas e limitações

- **C99 puro e sem bibliotecas externas:** usa apenas as APIs nativas de cada sistema.
- **Portabilidade:** todas as diferenças entre Linux e Windows ficam em blocos `#ifdef _WIN32` dentro de cada módulo, e a interface pública (`.h`) é a mesma para ambos.
- **IPv4 e IPv6:** a resolução usa `getaddrinfo` com `AF_UNSPEC`, então o programa aceita nomes e endereços literais de ambas as versões. Se um nome tiver endereços das duas famílias, é usado o primeiro que o sistema retornar.
- **Nomes de adaptadores no Windows:** são convertidos de UTF-16 para UTF-8, e o console é configurado para UTF-8.
- **Limitações:**
  - no máximo 64 endereços listados e 100 pacotes por medição;
  - intervalo fixo de 1 s entre pacotes;
  - *timeout* de 2 s por pacote;
  - redes ou *firewalls* que bloqueiam ICMP mostram 100% de perda, mesmo quando o host está acessível por outros protocolos.

---

## 9. Referências

### Obrigatórias

- CARISSIMI, Alexandre da Silva; ROCHOL, Juergen; GRANVILLE, Lisandro Zambenedetti. **Redes de computadores**. Porto Alegre: Bookman, 2009. 391 p.
- FOROUZAN, B. A. **Comunicação de Dados e Redes de Computadores**. 4. ed. São Paulo: McGraw-Hill, 2010.
- FOROUZAN, B. A.; MOSHARRAF, F. **Redes de Computadores: uma abordagem top-down**. São Paulo: McGraw-Hill, 2013.
- NINA, Edelweiss. **Algoritmos e programação com exemplos em Pascal e C**. Bookman, 2014.
- SANTOS, Marcelo G. **Algoritmos e Programação**. Porto Alegre: Sagah, 2018.

### Complementares

- COMER, Douglas E. **Interligação de redes com TCP/IP: princípios, protocolos e arquitetura**. 6. ed. Rio de Janeiro: Elsevier, 2015. ISBN 9788535278637.
- DASGUPTA, Sanjoy. **Algoritmos**. Porto Alegre: AMGH, 2010.
- PERES, André. **Redes de computadores II: Níveis de transporte e de rede**. Porto Alegre: Bookman, 2014.
- SCHMITT, Marcelo Augusto Rauh. **Redes de computadores: nível de aplicação e instalação de serviços**. Bookman, 2013.
- SEBESTA, R. W. **Conceitos de linguagem de programação**. 9. ed. Porto Alegre: Bookman, 2011. 792 p. ISBN 9788577807918.

### Especificações técnicas

- RFC 791: Internet Protocol (IPv4).
- RFC 792: Internet Control Message Protocol (ICMP).
- RFC 1918: Address Allocation for Private Internets.
- RFC 4291: IP Version 6 Addressing Architecture.
- RFC 4443: ICMPv6 for IPv6.
- RFC 8200: Internet Protocol, Version 6 (IPv6) Specification.

---

**Autora:** Joana Mespaque. Projeto Integrador II-A, UCPel.
