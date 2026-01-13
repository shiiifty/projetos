# This is the Python script for your project

def cria_intersecao(col, lin):
    """
    cria intersecao: str x int 7→ intersecao
    Cria uma interseção válida a partir de uma coluna e uma linha.

    Argumentos:
        col (str): Uma letra que representa a coluna da interseção (de 'A' a 'S').
        lin (int): Um número inteiro que representa a linha da interseção (de 1 a 19).

    Returns:
        tuple: Um tupl que contem a coluna e a linha da interseção.

    Raises:
        ValueError: Se os argumentos não estiverem dentro dos limites válidos.

    """

    if not (type(col) == str and type(lin) == int):
        raise ValueError ('cria_intersecao: argumentos invalidos')
    elif len(col) != 1:
        raise ValueError('cria_intersecao: argumentos invalidos')
    elif not ('A' <= str(col) <= 'S' and 1 <= lin <= 19):
        raise ValueError('cria_intersecao: argumentos invalidos')
    
    return (col, lin)

def obtem_col(i):
    '''
    obtem col: intersecao 7→ str
    Obtém a coluna de uma interseção a partir da sua representação de tuplo.

    Args:
        i (tuplo): Um tuplo que representa uma interseção (col, lin).

    Returns:
        str: Uma string que representa a coluna da interseção.
    
    '''

    return i[0]

def obtem_lin(i):
    '''
    obtem lin: intersecao 7→ int
    Obtém a linha de uma interseção a partir da sua representação de tuplo.

    Args:
        i (tuplo): Um tuplo rque representa uma interseção (col, lin).

    Returns:
        int: Um número inteiro que representa a linha da interseção.
    '''

    return i[1]

def eh_intersecao(arg):
    '''
    eh intersecao: universal 7→ booleano
    Verifica se um argumento é uma representação válida de uma interseção.

    Args:
        arg (tuplo): Um tuplo que representa uma interseção (col, lin).

    Returns:
        bool: True se o argumento for uma interseção, False caso contrário.
    '''


    if not isinstance(arg, tuple):
        return False
    if not len(arg) == 2:
        return False
    coluna = obtem_col(arg)
    linha = obtem_lin(arg)
    if not (type(coluna) == str and type(linha) == int):
        return False
    if not('A' <= coluna <= 'S' and 1 <= linha <= 19):
        return False
    return True

def intersecoes_iguais(i1, i2):
    '''
    intersecoes iguais: universal x universal 7→ booleano
    Verifica se duas interseções são iguais.

    Args:
        i1 (tuplo): Um tuplo que representa a primeira interseção (col, lin).
        i2 (tuplo): Um tuplo que representa a segunda interseção (col, lin).

    Returns:
        bool: True se as interseções forem iguais, False caso contrário.
    '''

    return obtem_col(i1) == obtem_col(i2) and obtem_lin(i1) == obtem_lin(i2)

def intersecao_para_str(i):
    '''
    intersecao para str : intersecao 7→ str
    Converte uma interseção numa representação de string.

    Args:
        i (tuplo): Um tuplo que representa a interseção (col, lin).

    Returns:
        str: Uma string que representa a interseção.
    '''
    return str(i[0]) + str(i[1])

def str_para_intersecao(s):
    '''
    str para intersecao: str 7→ intersecao
    Converte uma representação de string numa interseção.

    Args:
        s (str): Uma string que representa uma interseção no formato de "colunalinha".

    Returns:
        tuple: Uma tupla que representa a interseção no formato (col, lin).
    '''

#Caso em que a string tem dois caracteres, ou seja, uma letra e um numero    
    if len(s) == 2:
        return (str(s[0]), int(s[1]))
#Caso em que a string tem tres caracteres, ou seja, uma letra e dois numeros    
    elif len(s) == 3:
        return (str(s[0]), int(s[1] + s[2]))

def obtem_intersecoes_adjacentes(i, l):
    '''
    obtem intersecoes adjacentes: intersecao x intersecao 7→ tuplo
    Obtém as interseções adjacentes a uma interseção dada.

    Args:
        i (tuplo): Um tuplo que representa a interseção alvo.
        l (tuplo): Um tuplo que representa a interseção limite (que define o tabuleiro).

    Returns:
        tuple: Uma tupla contendo as interseções adjacentes.
    '''

    coluna_max = ord(obtem_col(l)) - 65
    linha_max = obtem_lin(l)
    colunai = ord(obtem_col(i)) - 65
    linhai = obtem_lin(i)
    adjacentes = []

#Se a intersecao dada nao estiver na primeira linha 
    if linhai > 1:
        adjacentes.append((chr(colunai + 65), linhai - 1))
#Se a intersecao dada nao estiver na primeira coluna 
    if colunai > 0:
        adjacentes.append((chr(colunai - 1 + 65), linhai))
#Se a intersecao dada nao estiver na ultima coluna 
    if colunai < coluna_max:
        adjacentes.append((chr(colunai + 1 + 65), linhai))
    
#Se a intersecao dada nao estiver na ultima linha 
    if linhai < linha_max:
        adjacentes.append((chr(colunai + 65), linhai + 1))

    return tuple(adjacentes)

def ordena_intersecoes(t):
    '''
    ordena intersecoes: tuplo 7→ tuplo
    Ordena um tuplo de interseções por ordem crescente.

    Args:
        t (tuplo): Um tupla que contem interseções (col, lin).

    Returns:
        tuplo: Um tuplao que contem as interseções ordenadas.
    '''

#Transformar o argumento dado em lista    
    lista_intersecoes = list(t)
    n = len(lista_intersecoes)

    for i in range(n):
        troca = False
#Dois ciclos para ser possivel comparar cada tuplo a todos os outros        
        for j in range(0, n - i - 1):
            intersecao1 = lista_intersecoes[j]
            intersecao2 = lista_intersecoes[j + 1]
#Se o numero de um tuplo for maior que o numero do outro ou no caso dos numeros serem iguais e se a letra de um for maior que a do outro entao troca-se um com o outro
            if intersecao1[1] > intersecao2[1] or (intersecao1[1] == intersecao2[1] and intersecao1[0] > intersecao2[0]):
                lista_intersecoes[j], lista_intersecoes[j + 1] = lista_intersecoes[j + 1], lista_intersecoes[j]
                troca = True
#Caso em que o tuplo ja esta ordenado        
        if not troca:
            break
            
    tuplo_ordenado = tuple(lista_intersecoes)
    return tuplo_ordenado

def cria_pedra_branca():
    '''
    cria pedra branca: {} 7→ pedra
    Cria uma pedra branca.

    Args:
        Nenhum.

    Returns:
        dict: Um dicionário que representa uma pedra branca com a chave 'cor' definida como 'O'.
    '''

    return {'cor': 'O'}

def cria_pedra_preta():
    '''
    cria pedra preta: {} 7→ pedra
    Cria uma pedra preta.

    Args:
        Nenhum.

    Returns:
        dict: Um dicionário que representa uma pedra preta com a chave 'cor' definida como 'X'.
    '''
    return {'cor': 'X'}

def cria_pedra_neutra():
    '''
    cria pedra neutra: {} 7→ pedra
    Cria uma pedra neutra.

    Args:
        Nenhum.

    Returns:
        dict: Um dicionário que representa uma pedra neutra com a chave 'cor' definida como '.'.
    '''
    return {'cor': '.'}

def eh_pedra(arg):
    '''
    eh pedra: universal 7→ booleano
    Verifica se o argumento é uma pedra válida.

    Args:
        arg (dict): Um dicionário que representa uma pedra com a chave 'cor'.

    Returns:
        bool: True se o valor atribuído à chave 'cor' for 'O', 'X' ou '.', caso contrário, False.
    '''
#Retornara true se a o valor atribuido a chave 'cor' for 'O', 'X' ou '.' , caso contrario false
    return arg['cor'] in {'O', 'X', '.'}

def eh_pedra_branca(p):
    '''
    eh pedra branca: pedra 7→ booleano
    Verifica se a pedra fornecida é uma pedra branca.

    Args:
        p (dict): Um dicionário que representa uma pedra com a chave 'cor'.

    Returns:
        bool: True se o valor atribuído à chave 'cor' for 'O' (branco), caso contrário, False.
    '''
#Retornara true se o valor atribuido a chave 'cor' for 'O'
    return p['cor'] == 'O'

def eh_pedra_preta(p):
    '''
    eh pedra preta: pedra 7→ booleano
    Verifica se a pedra fornecida é uma pedra preta.

    Args:
        p (dict): Um dicionário que representa uma pedra com a chave 'cor'.

    Returns:
        bool: True se o valor atribuído à chave 'cor' for 'X' (preto), caso contrário, False.
    '''
#Retornara true se o valor atribuido a chave 'cor' for 'X'
    return p['cor'] == 'X'

def pedras_iguais(p1, p2):
    '''
    pedras iguais: universal x universal 7→ booleano
    Verifica se duas pedras são iguais.

    Args:
        p1 (dict): Um dicionário que representa a primeira pedra com a chave 'cor'.
        p2 (dict): Um dicionário que representa a segunda pedra com a chave 'cor'.

    Returns:
        bool: True se as pedras forem iguais (mesma cor), caso contrário, False.
    '''
    return p1['cor'] == p2['cor']

def pedra_para_str(p):
    '''
    pedra para str : pedra 7→ str
    Converte uma pedra em uma representação de string.

    Args:
        p (dict): Um dicionário que representa uma pedra com a chave 'cor'.

    Returns:
        str: Uma string que representa a cor da pedra.

    Exemplo:
        pedra_para_str({'cor': 'O'}) retorna 'O'
    '''
    return str(p['cor'])

def eh_pedra_jogador(p):
    '''
    eh pedra jogador : pedra 7→ booleano
    Verifica se a pedra fornecida pertence a um jogador (branca ou preta).

    Args:
        p (dict): Um dicionário que representa uma pedra com a chave 'cor'.

    Returns:
        bool: True se o valor atribuído à chave 'cor' for 'O' (branca) ou 'X' (preta), caso contrário, False.
    '''
#Retornara true se o valor atribuido a chave 'cor' for 'O' ou 'X'
    return p['cor'] in ('O', 'X')

def cria_goban_vazio(n):
    '''
    cria goban vazio: int 7→ goban
    Cria um tabuleiro de Go vazio com o tamanho especificado.

    Args:
        n (int): Tamanho do tabuleiro, que tem ser 9, 13 ou 19.

    Returns:
        lista: Uma lista de listas que representa um tabuleiro de Go vazio, onde cada posição contém '.'.

    Raises:
        ValueError: Se o argumento n não for um número inteiro ou não for um dos valores permitidos.
    '''
#Verificar se n é um inteiro
    if not isinstance(n, int):
        raise ValueError('cria_goban_vazio: argumento invalido')
#Verificar se n toma o valor de 9 ou 13 ou 19
    if n not in (9, 13, 19):
        raise ValueError('cria_goban_vazio: argumento invalido')   
    lista = []
#Com este ciclo e adicionado a lista n listas constituidas por '.' cada lista tera n*'n' uma vez que os gobans são tabuleiros quadrados 
    for i in range(1, n+1):
        tuplo = ['.']*n
        lista += [tuplo]
    
    return lista

def cria_goban(n, ib, ip):
    '''
    cria goban: int x tuplo x tuplo 7→ goban
    Cria um tabuleiro de Go (goban) com base nos parâmetros especificados.

    Args:
        n (int): Tamanho do tabuleiro de Go, deve ser um dos valores: 9, 13 ou 19.
        ib (tuplo): Um tuplo de interseções marcadas com 'O'.
        ip (tuplo): Um tuplo de interseções marcadas com 'X'.

    Returns:
        lista: Uma lista que representa o tabuleiro de Go cujos elementos são 'O' e 'X' e '.'.

    Raises:
        ValueError: Se os argumentos fornecidos forem inválidos, como se n não for um inteiro,
        se n não for um dos tamanhos permitidos, se ib e ip não forem tuplos, se houver interseções comuns
        em ib e ip, se houver interseções repetidas em ib ou ip, ou se os elementos em ib e ip não forem interseções válidas.
    '''
#verificar se n é um inteiro
    if not isinstance(n, int):
        raise ValueError('cria_goban: argumentos invalidos')
#Verificar se n toma o valor de 9 ou 13 ou 19
    if n not in (9, 13, 19):
        raise ValueError('cria_goban: argumentos invalidos')
#verificar se ib e ip são tuplos
    if not(isinstance(ib, tuple) or isinstance(ip, tuple)):
        raise ValueError('cria_goban: argumentos invalidos')
    for intersecao in ib:
        for inter in ip:
            if intersecao == inter:
                raise ValueError('cria_goban: argumentos invalidos')
            
#Verificar se alguma intersecao de ib se repete       
    for i in range(len(ib)):
        for j in range(i + 1, len(ib)):
            if ib[i] == ib[j]:
                raise ValueError('cria_goban: argumentos invalidos')
            
#Verificar se alguma intersecao de ip se repete
    for i in range(len(ip)):
        for j in range(i + 1, len(ip)):
            if ip[i] == ip[j]:
                raise ValueError('cria_goban: argumentos invalidos')
            
    goban = cria_goban_vazio(n)
    for intersecao in ib:
#Verificar se os elementos de ib sao intersecoes
        if not eh_intersecao(intersecao):
            raise ValueError('cria_goban: argumentos invalidos')
        indice_tuplo = ord(intersecao[0]) - 65
        indice_numero = intersecao[1] - 1
#Para cada intersecao de ib mudar o elemento de g para 'O'
        goban[indice_tuplo][indice_numero] = 'O'

    for intersecao in ip:
#Verificar se os elementos de ip sao intersecoes
        if not eh_intersecao(intersecao):
            raise ValueError('cria_goban: argumentos invalidos')
        indice_tuplo = ord(intersecao[0]) - 65
        indice_numero = intersecao[1] - 1
#Para cada intersecao de ip mudar o elemento de g para 'X'
        goban[indice_tuplo][indice_numero] = 'X'

    return goban

def cria_copia_goban(t):
    '''
    cria copia goban: goban 7→ goban
    Cria uma cópia de um tabuleiro de Go (goban).

    Args:
        t (lista): O tabuleiro de Go original em lista.

    Returns:
        list: Uma cópia do tabuleiro de Go original.

    Esta função recebe o tabuleiro de Go original 't' e cria uma nova cópia dele,
    preservando a mesma estrutura e conteúdo. O tabuleiro original não é modificado.
    '''

    copia_goban = []
    for linha in t:
        copia_linha = list(linha)
        copia_goban.append(copia_linha)
    
    return copia_goban
    
def obtem_ultima_intersecao(g):
    '''
    obtem ultima intersecao: goban 7→ intersecao
    Obtém a última interseção em um tabuleiro de Go (goban).

    Args:
        g (lista): O tabuleiro de Go representado em uma lista.

    Returns:
        tuple: Uma interseção que representa a última linha e coluna do tabuleiro.

    Esta função determina a última interseção em um tabuleiro de Go com base no número de elementos nas tuplas
    e no número de tuplas no tabuleiro. A última interseção é retornada como uma tupla contendo a letra da coluna
    e o número da linha correspondentes.
    '''
#A ultima linha do goban sera o numero de elementos dos tuplos de g
#A ultima coluna de g sera o numero de tuplos de g
    for tuplo in g:
        letra = chr(64 + len(g))
        numero = len(tuplo)

    return (letra, numero)
        
def obtem_pedra(g, i):
    '''
    obtem pedra: goban x intersecao 7→ pedra
    Obtém a pedra em uma interseção específica de um tabuleiro de Go.

    Args:
        g (lista): O tabuleiro de Go representado como uma lista.
        i (tuplo): A interseção específica representada como um tupl cque contem a letra da coluna e o número da linha.

    Returns:
        dict: Um dicionário que contem a cor da pedra na interseção ('O' para preta, 'X' para branca ou '.' para neutra).

    Esta função recebe um tabuleiro de Go e uma interseção específica, e determina a cor da pedra nessa interseção.
    A cor da pedra é retornada em forma de dicionário, com a chave 'cor' indicando 'O' para preta, 'X' para branca ou '.' para neutra.
    '''

    indice_tuplo = ord(i[0]) - 65
    indice_numero = i[1] - 1
    if g[indice_tuplo][indice_numero] == 'O':
        return {'cor': 'O'}
    elif g[indice_tuplo][indice_numero] == 'X':
        return {'cor': 'X'}
    else:
        return {'cor': '.'}

def obtem_cadeia(g, i):
    '''
    obtem cadeia: goban x intersecao 7→ tuplo
    Obtém a cadeia de pedras em uma interseção específica de um tabuleiro de Go.

    Args:
        g (lista): O tabuleiro de Go representado como uma lista.
        i (tuplo): A interseção específica representada como um tuplo que contem a letra da coluna e o número da linha.

    Returns:
        tuple: Um tuplo que contem as interseções que fazem parte da cadeia de pedras na interseção especificada.

    Esta função recebe um tabuleiro de Go e uma interseção específica e determina a cadeia de pedras a que a interseção pertence.
    A cadeia é representada como um tuplo de interseções que fazem parte da cadeia. A função considera a cor das pedras (preta, branca ou neutra)
    e identifica as pedras que fazem parte da mesma cadeia na interseção especificada.
    '''
    l = obtem_ultima_intersecao(g)
    cadeia = [i]
    intersecoes_adjacentes = list(obtem_intersecoes_adjacentes(i, l))
    #cadeia  de pedras brancas
    if eh_pedra_branca(obtem_pedra(g, i)):
        while len(intersecoes_adjacentes) > 0:
            for intersecao in intersecoes_adjacentes:
                if eh_pedra_branca(obtem_pedra(g, intersecao)) and intersecao not in cadeia:
                    cadeia.append(intersecao)
                    b = obtem_intersecoes_adjacentes(intersecao, l)
                    for adjacente in b:
                        if adjacente not in intersecoes_adjacentes:
                            intersecoes_adjacentes.append(adjacente)


            intersecoes_adjacentes = intersecoes_adjacentes[1:]

    #cadeia de pedras pretas
    if eh_pedra_preta(obtem_pedra(g, i)):
        while len(intersecoes_adjacentes) > 0:
            for intersecao in intersecoes_adjacentes:
                if eh_pedra_preta(obtem_pedra(g, intersecao)) and intersecao not in cadeia:
                    cadeia.append(intersecao)
                    b = obtem_intersecoes_adjacentes(intersecao, l)
                    for adjacente in b:
                        if adjacente not in intersecoes_adjacentes:
                            intersecoes_adjacentes.append(adjacente)


            intersecoes_adjacentes = intersecoes_adjacentes[1:]

    #cadeia de pedras neutras
    if (obtem_pedra(g, i)) == {'cor': '.'}:
        while len(intersecoes_adjacentes) > 0:
            for intersecao in intersecoes_adjacentes:
                if (obtem_pedra(g, intersecao)) == {'cor': '.'} and intersecao not in cadeia:
                    cadeia.append(intersecao)
                    b = obtem_intersecoes_adjacentes(intersecao, l)
                    for adjacente in b:
                        if adjacente not in intersecoes_adjacentes:
                            intersecoes_adjacentes.append(adjacente)


            intersecoes_adjacentes = intersecoes_adjacentes[1:]

    return ordena_intersecoes(cadeia)

def coloca_pedra(g, i, p):
    '''
    coloca pedra: goban x intersecao x pedra 7→ goban
    Coloca uma pedra numa interseção específica de um tabuleiro de Go.

    Args:
        g (lista): O tabuleiro de Go representado como uma lista bidimensional.
        i (tuplo): A interseção específica representada como um tupl que contem a letra da coluna e o número da linha.
        p (dicionario): A pedra a ser colocada, representada como um dicionário com a chave 'cor' indicando a cor ('O' para preta, 'X' para branca).

    Returns:
        list: O tabuleiro de Go atualizado após a colocação da pedra.

    Esta função recebe um tabuleiro de Go, uma interseção específica e uma pedra a ser colocada na interseção.
    Ela atualiza o tabuleiro de Go, alterando o elemento que corresponde à interseção 'i' para a cor da pedra 'p'.
    '''
    indice_tuplo = ord(i[0]) - 65
    indice_numero = i[1] - 1
#Mudar o elemento g que corresponde a interscao i para a pedra p
    g[indice_tuplo][indice_numero] = p['cor']
    return g
                    
def remove_pedra(g, i):
    '''
    remove pedra: goban x intersecao 7→ goban
    Remove uma pedra de uma interseção específica de um tabuleiro de Go.

    Args:
        g (lista): O tabuleiro de Go representado como uma lista.
        i (tuplo): A interseção específica representada como um tuplo que contem a letra da coluna e o número da linha.

    Returns:
        list: O tabuleiro de Go atualizado após a remoção da pedra na interseção.

    Esta função recebe um tabuleiro de Go e uma interseção específica. Remove a pedra da interseção, atualizando o tabuleiro
    ao alterar o elemento que corresponde à interseção 'i' para uma pedra neutra ('.').

    '''
    indice_tuplo = ord(i[0]) - 65
    indice_numero = i[1] - 1
#Mudar o elemento g que corresponde a interscao i para '.'
    g[indice_tuplo][indice_numero] = '.'
    return g

def remove_cadeia(g, t):
    '''
    remove cadeia: goban x tuplo 7→ goban
    Remove uma cadeia de pedras de um tabuleiro de Go.

    Args:
        g (lista): O tabuleiro de Go representado como uma lista.
        t (tuplo): Um tuplo que contem as interseções que fazem parte da cadeia de pedras a ser removida.

    Returns:
        list: O tabuleiro de Go atualizado após a remoção da cadeia de pedras.

    Esta função recebe um tabuleiro de Go e um tuplo que contem interseções que fazem parte de uma cadeia de pedras.
    Ela remove todas as pedras que fazem parte da cadeia, atualizando o tabuleiro, alterando os elementos correspondentes
    '''
#Remover todas as pedras o tuplo t
    for i in t:
        g = remove_pedra(g, i)
    return g

def eh_goban(arg):
    '''
    eh goban: universal 7→ booleano
    Verifica se o argumento é um tabuleiro de Go (goban) válido.

    Args:
        arg (universal): O argumento a ser verificado.

    Returns:
        bool: True se o argumento é um goban válido, False caso contrário.

    Esta função verifica se o argumento fornecido é um tabuleiro de Go válido, seguindo as regras:
    1. Deve ser uma lista.
    2. O tamanho do tabuleiro deve ser 9x9, 13x13 ou 19x19.
    3. O tabuleiro deve ser quadrado, com todos os elementos da mesma lista tendo o mesmo comprimento.
    4. Os elementos do tabuleiro devem ser apenas '.', 'X' (para pedras brancas) ou 'O' (para pedras pretas).

    Retorna True se o argumento for um goban válido, e False caso contrário.
    '''
#Verificar se  o argumento e uma lista
    if not(isinstance(arg, list)):
        return False
#Vericar se o comprimento do argumento toma o valor de 9 ou 13 ou 19
    if len(arg) not in (9, 13, 19):
        return False
#Uma vez que os gobans sao quadrados o tamanhos dos elementos de g tem de ser igual ao comprimento de g    
    for tuplo in arg:
        if not(len(arg) == len(tuplo)):
            return False
#Uma vez que todos os elementos de g tem de se pedras brancas ou pretas ou neutras         
        for elemento in tuplo:
            if not(elemento == '.' or elemento == 'X' or elemento == 'O'):
                return False
    return True

def eh_intersecao_valida(g, i):
    '''
    eh intersecao valida: goban x intersecao 7→ booleano
    Verifica se uma interseção é válida em um tabuleiro de Go (goban) específico.

    Args:
        g (lista): O tabuleiro de Go representado como uma lista bidimensional.
        i (tuplo): A interseção a ser verificada, representada como um tuplo que contem a letra da coluna e o número da linha.

    Returns:
        bool: True se a interseção for válida no tabuleiro de Go, False caso contrário.

    Esta função verifica se uma interseção específica é válida em um tabuleiro de Go, seguindo as regras:
    1. O tabuleiro (goban) deve ser um goban válido (usando a função eh_goban).
    2. A interseção deve ser representada como um tuplo.
    3. A interseção deve estar dentro dos limites do tabuleiro de Go.

    Retorna True se a interseção for válida no tabuleiro de Go, e False caso contrário.
    '''
    if not eh_goban(g):
        return False
    if not isinstance(i, tuple):
        return False
    
    ultima_intersecao = obtem_ultima_intersecao(g)
#Verificar se a intersecao dada esta dentro do goban
    if not('A' <= i[0] <= ultima_intersecao[0]):
        return False
    if not (1 <= i[1] <= ultima_intersecao[1]):
        return False
    return True

def gobans_iguais(g1, g2):
    '''
    gobans iguais: universal x universal 7→ booleano
    Verifica se dois tabuleiros de Go (gobans) são iguais.

    Args:
        g1 (universal): O primeiro tabuleiro de Go a ser comparado.
        g2 (universal): O segundo tabuleiro de Go a ser comparado.

    Returns:
        bool: True se os tabuleiros de Go são iguais, False caso contrário.

    Esta função verifica se dois tabuleiros de Go são idênticos, seguindo as regras:
    1. Pelo menos um dos argumentos deve ser um goban válido (usando a função eh_goban).
    2. Os dois tabuleiros de Go são iguais se tiverem a mesma configuração, ou seja, todas as interseções são idênticas.

    Retorna True se os tabuleiros de Go são iguais, e False caso contrário.
    '''
    if not(eh_goban(g1) or eh_goban(g2)):
        return False
    
    return g1 == g2

def goban_para_str(g):
    '''
    goban para str : goban 7→ str
    Converte um tabuleiro de Go (goban) num tabuleiro, ou seja, numa representação em string.

    Args:
        g (lista): O tabuleiro de Go representado como uma lista.
    Returns:
        str: Uma representação em string do tabuleiro de Go.

    Esta função converte um tabuleiro de Go em uma representação em formato de string, seguindo as regras:
    - O tabuleiro é representado como uma lista 'g'.
    - A representação inclui o tabuleiro com letras (colunas) e números (linhas), além das pedras 'X' (brancas), 'O' (pretas) e espaços vazios '.'.

    Retorna a representação em string do tabuleiro de Go.
    '''
    caminhos_verticais = len(g)
    caminhos_horizontais = len(g[0])
#Caso do goban quando n é 9    
    if caminhos_horizontais < 10 and caminhos_horizontais > 0:
        goban = '  '
#Escrever primeira linha de letras         
        for letra in range(caminhos_verticais):
            goban += ' ' + chr(65 + letra)
        
        goban += '\n'
#Escrever o numero correspondente a linha 
        for numero in range(caminhos_horizontais, 0, -1):
            goban += ' ' + str(numero)
#Escrever todos os elementos da linha de g
            for letra in range(caminhos_verticais):
                if g[letra][numero-1] == 'X':
                    goban += ' X'
                elif g[letra][numero-1] == 'O':
                    goban += ' O'
                else:
                    goban += ' .'
#Escrever o numero correspondente a linha do outro lado
            goban += '  ' + str(numero)
            goban += '\n'

        goban += '  '
#Escrever a ultima linha de letras
        for letra in range(caminhos_verticais):
            goban += ' ' + chr(65 + letra)
    
    else:
#Caso do goban quando n é 13 ou 19
        goban = '  '
#Escrever primeira linha de letras
        for letra in range(caminhos_verticais):
            goban += ' ' + chr(65 + letra)
        
        goban += '\n'

#Escrever o numero correspondente a linha 
        for numero in range(caminhos_horizontais, 0, -1):
            if numero >= 10:
                goban += str(numero)
                for letra in range(caminhos_verticais):
#Escrever todos os elementos da linha de g
                    if g[letra][numero-1] == 'X':
                        goban += ' X'
                    elif g[letra][numero-1] == 'O':
                        goban += ' O'
                    else:
                        goban += ' .'
#Escrever o numero correspondente a linha do outro lado
                goban += ' ' + str(numero)
                goban += '\n'

            else:
                goban += ' ' + str(numero)
#Escrever o numero correspondente a linha 
                for letra in range(caminhos_verticais):
#Escrever todos os elementos da linha de g
                    if g[letra][numero-1] == 'X':
                        goban += ' X'
                    elif g[letra][numero-1] == 'O':
                        goban += ' O'
                    else:
                        goban += ' .'
#Escrever o numero correspondente a linha do outro lado
                goban += '  ' + str(numero)
                goban += '\n'



        goban += '  '
#Escrever a ultima linha de letras
        for letra in range(caminhos_verticais):
            goban += ' ' + chr(65 + letra)


    return goban

def obtem_territorios(g):
    '''
    obtem territorios: goban 7→ tuplo
    Obtém os territórios do tabuleiro de Go (goban).

    Args:
        g (lista): O tabuleiro de Go representado como uma lista.

    Returns:
        tuple: Um tuplo que contem as cadeias de interseções que representam os territórios vazios.

    Esta função verifica o tabuleiro de Go em busca de territórios vazios (interseções sem pedras pretas ou brancas) e retorna uma lista de cadeias que representam esses territórios.

    Retorna um tuplo que contem as cadeias de interseções que representam os territórios vazios do tabuleiro de Go.
    '''
    territorios = []  
    for i in range(len(g)):
        for j in range(len(g[0])):
#Para cada pedra neutra de g obter cadeia dessa pedra             
            if g[i][j] == '.':
                letra = chr(65 + i)
                numero = j + 1
                cadeia = obtem_cadeia(g, (letra, numero))
#Se a cadeia nao estiver em cadeia adicionar 
                if cadeia not in territorios:
                    territorios.append(cadeia)

    return territorios

def obtem_adjacentes_diferentes(g, t):
    '''
    obtem adjacentes diferentes: goban x tuplo 7→ tuplo
    Obtém interseções adjacentes com pedras de jogadores diferentes em relação a uma cadeia de interseções.

    Args:
        g (lista): O tabuleiro de Go representado como uma lista.
        t (tuplo): Uma cadeia de interseções a ser considerada.

    Returns:
        tuple: Um tuplo contendo as interseções adjacentes com pedras de jogadores diferentes.

    Esta função recebe um tabuleiro de Go e uma cadeia de interseções (t) e retorna as interseções adjacentes a essa cadeia que contêm pedras de jogadores diferentes. 
    A função considera tanto interseções com pedras vivas quanto interseções vazias.

    Retorna um tuplo que contem as interseções adjacentes com pedras de jogadores diferentes em relação à cadeia de interseções fornecida.

    '''
    l = obtem_ultima_intersecao(g)
    cadeia = []
    for tuplo in t:
#caso em que a o tuplo dado e de pedras lives
        if (obtem_pedra(g, tuplo)) == {'cor': '.'}:
            intersecoes_adjacentes = obtem_intersecoes_adjacentes(tuplo, l)
            for adjacente in intersecoes_adjacentes:
                if eh_pedra_jogador(obtem_pedra(g, adjacente)) and adjacente not in cadeia:
                    cadeia.append(adjacente)
#Caso em que o tuplo dado e de pedras de jogador
        elif eh_pedra_jogador(obtem_pedra(g, tuplo)):
            intersecoes_adjacentes = obtem_intersecoes_adjacentes(tuplo, l)
            for adjacente in intersecoes_adjacentes:
                if (obtem_pedra(g, adjacente)) == {'cor': '.'} and adjacente not in cadeia:
                    cadeia.append(adjacente)

    return ordena_intersecoes(cadeia)    

def jogada(g, i, p):
    '''
    jogada: goban x intersecao x pedra 7→ goban
    Obtém interseções adjacentes com pedras de jogadores diferentes em relação a uma cadeia de interseções.

    Args:
        g (lista): O tabuleiro de Go representado como uma lista.
        t (tuplo): Uma cadeia de interseções a ser considerada.

    Returns:
        tuple: Um tuplo que contem as interseções adjacentes com pedras de jogadores diferentes.

    Esta função recebe um tabuleiro de Go e uma cadeia de interseções (t) e retorna as interseções adjacentes a essa cadeia que contêm pedras de jogadores diferentes. A função considera tanto interseções com pedras de jogador quanto pedras neutras.

    Retorna um tuplo que contem as interseções adjacentes com pedras de jogadores diferentes em relação à cadeia de interseções fornecida.
    '''
    l = obtem_ultima_intersecao(g)
    intersecoes_adjacentes = obtem_intersecoes_adjacentes(i, l)

#Caso em que a pedra colocada e branca
    if p['cor'] == 'O':
        for intersecao in intersecoes_adjacentes:
#Se houver algum pedra preta adjacente a pedra colocada
            if eh_pedra_preta(obtem_pedra(g, intersecao)):
#Obter a cadeia dessa pedra
                cadeia = obtem_cadeia(g, intersecao)
#Obter todas as intersecoes adjacentes a cadeia
                if obtem_adjacentes_diferentes(g, cadeia) == (i,):
                    remove_cadeia(g, cadeia)
                
#Caso em que a pedra colocada e preta
    elif p['cor'] == 'X':
        for intersecao in intersecoes_adjacentes:
#Se houver algum pedra branca adjacente a pedra colocada
            if eh_pedra_branca(obtem_pedra(g, intersecao)):
#Obter a cadeia dessa pedra
                cadeia = obtem_cadeia(g, intersecao)
#Obter todas as intersecoes adjacentes a cadeia 
                if obtem_adjacentes_diferentes(g, cadeia) == (i,):
                    remove_cadeia(g, cadeia)
                
    coloca_pedra(g, i, p)
    return g

def obtem_pedras_jogadores(g):
    '''
    obtem pedras jogadores: goban 7→ tuplo
    Obtém o número de pedras dos jogadores (brancas e pretas) em um tabuleiro de Go (goban).

    Args:
        g (lista): O tabuleiro de Go representado como uma lista.
    Returns:
        tuple: Um tuplo que contem o número de pedras brancas e o número de pedras pretas no tabuleiro.

    Esta função conta o número de pedras dos jogadores (brancas e pretas) em um tabuleiro de Go. Ela itera por todos os elementos do tabuleiro e conta as pedras de cada jogador.

    Retorna um tuplo com o número de pedras brancas e o número de pedras pretas no tabuleiro.
    '''
    numero_pedras_brancas = 0
    numero_pedras_pretas = 0

#Iterar por todos os elementos de g
    for tuplo in g:
        for elemento in tuplo:
#Se o elemento for 'O' adicionar 1 ao valor de pedras brancas
            if elemento == 'O':
                numero_pedras_brancas += 1
#Se o elemento for 'X' adicionar 1 ao valor de pedras pretas
            elif elemento == 'X':
                numero_pedras_pretas += 1

    return (numero_pedras_brancas, numero_pedras_pretas)

def calcula_pontos(g):
    '''
    calcula pontos: goban 7→ tuple
    Calcula os pontos dos jogadores (brancos e pretos) em um tabuleiro de Go (goban).

    Args:
        g (lista): O tabuleiro de Go representado como uma lista.

    Returns:
        tuple: Um tuplo que contem o número de pontos das pedras brancas e o número de pontos das pedras pretas.

    Esta função calcula os pontos dos jogadores em um tabuleiro de Go, considerando as pedras e territórios cercados. Ela itera por todos os elementos do tabuleiro e conta os pontos das pedras brancas e pretas.

    Retorna um tuplo com o número de pontos das pedras brancas e o número de pontos das pedras pretas no tabuleiro.
    '''
    numero_pontos_branco = 0
    numero_pontos_preto = 0
#Iterar por todos os elementos de g
    for tuplo in g:
        for elemento in tuplo:
#Se o elemento for 'O' adicionar 1 ao valor de pontos do jogador das pedras brancas
            if elemento == 'O':
                numero_pontos_branco += 1
#Se o elemento for 'X' adicionar 1 ao valor de pontos do jogador das pedras pretas
            elif elemento == 'X':
                numero_pontos_preto += 1
            
    territorios = obtem_territorios(g)
    for territorio in territorios:
        intersecoes_diferentes = obtem_adjacentes_diferentes(g, territorio)
#Verificar se o goban é vazio 
        if intersecoes_diferentes == ():
            return (numero_pontos_branco, numero_pontos_preto)
#Verificar se o territorio e cercado so por pedras brancas           
        if all(eh_pedra_branca(obtem_pedra(g, i)) for i in intersecoes_diferentes):
            numero_pontos_branco += len(territorio)
#Verificar se o territorio e cercado so por pedras pretas
        elif all(eh_pedra_preta(obtem_pedra(g, i)) for i in intersecoes_diferentes):
            numero_pontos_preto += len(territorio)
    
    return (numero_pontos_branco, numero_pontos_preto)

def eh_jogada_legal(g, i, p, l):
    '''
    eh jogada legal: goban x intersecao x pedra x goban 7→ booleano 
    Verifica se uma jogada é legal em um tabuleiro de Go.

    Args:
        g (lista): O tabuleiro de Go no estado atual.
        i (tuplo): A interseção onde a jogada será feita.
        p (dicionário): A pedra a ser colocada na interseção.
        l (lista): O estado do tabuleiro após a jogada (cópia do tabuleiro original).

    Returns:
        bool: True se a jogada for legal, False caso contrário.

    Esta função verifica se uma jogada é legal num tabuleiro de Go. Para ser considerada legal, a jogada deve atender aos seguintes critérios:

    1. A interseção `i` deve ser uma interseção válida no tabuleiro `g`.
    2. A interseção `i` deve estar vazia (ou seja, não ocupada por uma pedra).
    3. Após a jogada, o estado do tabuleiro não deve ser igual ao estado anterior `l` (ou seja, a jogada deve causar uma mudança no tabuleiro).
    4. A interseção onde a pedra é colocada não deve estar completamente cercada por pedras do jogador adversário.

    Retorna True se a jogada for legal e False caso contrário.
    '''
#Verificar se a intersecao dada e valida    
    if not eh_intersecao_valida(g, i):
        return False
    
    letra = ord(i[0]) - 65
    numero = i[1] - 1 

#Verificar se a intersecao dada é vazia
    if g[letra][numero] != '.':
        return False

    
#Verificar se após a jogada ser feita a peca tem liberdades    
    copia = cria_copia_goban(g)
    jogada(copia, i, p)
    cadeia = obtem_cadeia(copia, i)
    if obtem_adjacentes_diferentes(copia, cadeia) == ():
        return False

#Verificar se depois da jogada o goban é alterado
    copia2 = cria_copia_goban(g)
    jogada(copia2, i, p)
    if copia2 == l:
        return False
    
    return True

def turno_jogador(g, p, l):
    '''
    turno jogador: goban x pedra x goban 7→ booleano
    Realiza o turno de um jogador no jogo de Go.

    Args:
        g (lista): O tabuleiro de Go no estado atual.
        p (dict): O jogador atual (com a sua cor).
        l (lista): O estado do tabuleiro depois da jogada do oponente (cópia do tabuleiro original).

    Returns:
        bool: True se o jogador fez uma jogada válida, False se o jogador passou o turno.

    Esta função permite que um jogador realize o seu turno no jogo de Go. O jogador pode escolher fazer uma jogada numa interseção válida ou passar o seu turno. Para fazer uma jogada, o jogador deve fornecer uma interseção válida ou a entrada 'P' para passar o turno.

    A função verifica se a jogada é legal usando a função `eh_jogada_legal`. Se a jogada for legal, a pedra é colocada no tabuleiro. Se o jogador escolher passar o turno, a função retorna False. Caso contrário, retorna True.

    O jogador atual é representado pelo dicionário `p` que contem a sua cor ('O' para branco ou 'X' para preto).
    '''
    while True:
        jogada1 = input(f"Escreva uma intersecao ou 'P' para passar [{p['cor']}]:")
#Caso em que o jogador passa o seu turno
        if jogada1 == 'P':
            return False 
        else:
            intersecao = str_para_intersecao(jogada1)
#Verificar se a jogada é legal, se for colocar a pedra
            if eh_jogada_legal(g, intersecao, p, l):
                jogada(g, intersecao, p)
                return True 

def go(n, tb, tn):
    '''
    go: int x tuple x tuple 7→ booleano
    Permote jogar o jogo go por completo

    Args:
        n (inteiro): Tamanho do tabuleiro (9, 13 ou 19).
        tb (tuplo): Tuplo de interseções com pedras brancas (opcional).
        tn (tuplo): Tuplo de interseções com pedras pretas (opcional).

    Returns:
        bool: True se o jogador branco ganhar, False se o jogador preto ganhar.

    Esta função inicia uma partida de Go em um tabuleiro de tamanho `n`. Ela permite que o usuário configure o tabuleiro inicial, fornecendo as posições das pedras brancas (tb) e pretas (tn). Se nenhum dos jogadores realizar uma jogada válida após a outra, o jogo termina, e o jogador com mais pontos vence.

    A função verifica a validade dos argumentos (n, tb e tn) e inicializa o tabuleiro. Ela executa o jogo alternando os turnos entre o jogador preto (X) e o jogador branco (O). Os jogadores fazem as suas jogadas, e o jogo continua até ambos os jogadores passarem os seus turnos consecutivamente. Nesse momento, a função calcula os pontos de ambos os jogadores e retorna True se o jogador branco tiver mais pontos ou False se o jogador preto tiver mais pontos.

    A função imprime o estado do tabuleiro e os pontos de ambos os jogadores durante o jogo.
    '''
#Verificar se n é um numero inteiro
    if not type(n) == int:
        raise ValueError('go: argumentos invalidos')
#Verificar se n toma o valor de 9, 13 ou 19
    if n not in (9, 13, 19):
        raise ValueError('go: argumentos invalidos')
#Verificar se tb e tn são tuplos
    if not (isinstance(tb, tuple) or isinstance(tn, tuple)):
        raise ValueError('go: argumentos invalidos')
    g = cria_goban_vazio(n)
    if not tb == ():
        for i in tb:
            if not eh_intersecao_valida(g, i):
                raise ValueError('go: argumentos invalidos')
    if not tn == ():
        for j in tn:
            if not eh_intersecao_valida(g, j):
                raise ValueError('go: argumentos invalidos')
    goban = cria_goban(n, tb, tn)
    pontos = calcula_pontos(goban)
    pontos_branco = pontos[0]
    pontos_preto = pontos[1]
    print(f'Branco (O) tem {pontos_branco} pontos')
    print(f'Preto (X) tem {pontos_preto} pontos')
    print(goban_para_str(goban))
    a = True
    b = True
#Pedir jogadas até ambas terem sido passadas
    while a == True or b == True:
        turno_preto = turno_jogador(goban, cria_pedra_preta(), cria_copia_goban(goban))
        if turno_preto == False:
#Caso a jogada tenha sido passada trocar a para falso
            a = False
            pontos = calcula_pontos(goban)
            pontos_branco = pontos[0]
            pontos_preto = pontos[1]
            print(f'Branco (O) tem {pontos_branco} pontos')
            print(f'Preto (X) tem {pontos_preto} pontos')
            print(goban_para_str(goban))
        
        else:
            pontos = calcula_pontos(goban)
            pontos_branco = pontos[0]
            pontos_preto = pontos[1]
            print(f'Branco (O) tem {pontos_branco} pontos')
            print(f'Preto (X) tem {pontos_preto} pontos')
            print(goban_para_str(goban))
        
        turno_branco = turno_jogador(goban, cria_pedra_branca(), cria_copia_goban(goban))
        if turno_branco == False:
#Caso a jogada tenha sido passada trocar b para falso
            b = False
            pontos = calcula_pontos(goban)
            pontos_branco = pontos[0]
            pontos_preto = pontos[1]
            print(f'Branco (O) tem {pontos_branco} pontos')
            print(f'Preto (X) tem {pontos_preto} pontos')
            print(goban_para_str(goban))
        
        else:
            pontos = calcula_pontos(goban)
            pontos_branco = pontos[0]
            pontos_preto = pontos[1]
            print(f'Branco (O) tem {pontos_branco} pontos')
            print(f'Preto (X) tem {pontos_preto} pontos')
            print(goban_para_str(goban))
#Se as duas jogadas foram passadas verifiar quem foi o vencedor
        if a == False and b == False:
            pontos = calcula_pontos(goban)
            pontos_branco = pontos[0]
            pontos_preto = pontos[1]
            print(f'Branco (O) tem {pontos_branco} pontos')
            print(f'Preto (X) tem {pontos_preto} pontos')
            print(goban_para_str(goban))
            if pontos_branco >= pontos_preto:
                return True
            
            return False
            

go(9, (), ())
