# This is the Python script for your project

"""
Verifica se o argumento dado e um territorio
"""

def eh_territorio(t):
#Verificar se o territorio dado nao e vazio    
    if t == ():
        return False
#Verificar se o argumento e um tuplo    
    if not isinstance(t, tuple):
        return False
#Verificar se todos os elementos de t sao tuplos 
    for tuplos in t:
        if not isinstance(tuplos, tuple):
            return False
        
        tamanho_dos_tuplos = len(t[0])
#Verificar se todos os elementos de t tem o mesmo tamanho        
        if len(tuplos) != tamanho_dos_tuplos:
            return False
#Verificar se todos os elementos de cada tuplo de t e ou 0 ou 1
        for elementos in tuplos:
            if not isinstance(elementos, int):
                return False
            if elementos != 0 and elementos != 1:
                return False
#Verificar se o territorio tem no minimo uma coluna e no maximo 26(alfabeto portugues)   
    caminhos_verticais = len(t)
    caminhos_horizontais = len(t[0])
    if caminhos_verticais < 1 or caminhos_verticais > 26:
        return False
#Verificar se o territorio tem no minimo uma linha e no maximo 99
    if caminhos_horizontais < 1 or caminhos_horizontais > 99:
        return False
        
    return True

"""
Do territorio fornecido esta funcao vai-nos dar a ulrima intersecao, isto e, a intersecao que esta localizada no
canto superior direito do territorio
"""

def obtem_ultima_intersecao(t):
#Entrar dentro do tuplo t
    for tuplos in t:
#Letra correspondente ao ultimo tuplo de t 
        letra = chr(65+len(t)-1)
        tuplo = (letra, len(tuplos))
    return tuplo

"""
Esta funcao verifica se o argumento fornecido e uma intersecao
"""

def eh_intersecao(tuplo):
#Verificar se o argumento dado e um tuplo de dois elementos    
    if not isinstance(tuplo, tuple) or len(tuplo) != 2:
        return False
    
    letra = tuplo[0]
    numero = tuplo[1]

#verificar se o primeiro elemento de t e uma string e se o segundo e um inteiro
    if not(isinstance(letra, str) and isinstance(numero, int)):
        return False
#Verificar se a letra dada tem comprimento 1    
    if len(letra) != 1:
        return False
#verificar se a letra dada esta entre 'A' e 'Z'    
    if not ('A'<= letra <= 'Z'):
        return False
#Verificar se o numero dado esta entre 0 e 100
    if not (0 < numero <100):
        return False  
    
    return True

"""
Esta funcao verifica se o arumento dado é uma interscao possivel no territorio dado ou seja se a ultima intersecao do territorio
é ('E', 4) a intersecao ('F', 5) nao pode ser uma intersecao nesse territorio
"""

def eh_intersecao_valida(t, i):
#Verificar se t e um territorio valido e se i e uma intersecao    
    if (eh_territorio(t) == False or eh_intersecao(i) == False):
        return False
#Verificar se a coluna ou seja ord(letra correspondente a intersecao) esta entre 64 e o numero de colunas do territorio 
    if not (64 < ord(i[0]) < (64 + len(t) + 1)):
        return False
#Verificar se o numero da linha de i e menor ou igual ao numero de linhas do territorio     
    for tuplo in t:
        if i[1] > len(tuplo):
            return False
    return True

"""
Esta funcao vai verificar se a intersecao dada é ou não uma montanha no territorio tambem fornecido
"""

def eh_intersecao_livre(t, i):
#Verificar se i e uma intersecao    
    if not eh_intersecao_valida:
        return False
    letra = i[0]
    numero_tuplo = ord(letra) - 65
    tuplo = t[numero_tuplo] 
#Ver se a intersecao e uma montanha, se nao for e intersecao livre    
    if tuplo[i[1]-1] != 0:
        return False
    else:
        return True

"""
Esta duncao vai nos dar todas as intersecões que rodeiam a intersecao fornecida
"""

def obtem_intersecoes_adjacentes(t, i):
    if eh_intersecao_valida(t, i) == False:
        raise ValueError('obtem_intersecoes_adjacentes: argumentos invalidos')
    
    tuplo = []
    coluna, linha  = ord(i[0]) - 65, i[1]
 
    if linha > 0 and eh_intersecao_valida(t,(chr(65 + coluna), linha - 1)) == True :
        tuplo.append((chr(65 + coluna), linha - 1))
 
    if coluna > 0 and eh_intersecao_valida(t,(chr(65 + coluna - 1), linha)) == True:
        tuplo.append((chr(65 + coluna - 1), linha))

    if coluna < len(t) and eh_intersecao_valida(t,(chr(65 + coluna + 1), linha)) == True:
        tuplo.append((chr(65 + coluna + 1), linha))

    if linha < len(t[0]) and eh_intersecao_valida(t,(chr(65 + coluna), linha + 1)) == True:
        tuplo.append((chr(65 + coluna), linha + 1))

    return tuple(tuplo)

"""
Esta funcao vai-nos ordenar uma cadeia de intersecoes de acordo com o modo de leitura de intersecoes de um territorio
"""

def ordena_intersecoes(tup):
#Transformar o argumento dado em lista    
    lista_intersecoes = list(tup)
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

"""
Esta funcao da-nos o territorio ,dado em tuplo, em forma de string, ou seja, mais visual 
"""

def territorio_para_str(t):
    if eh_territorio == False:
        raise ValueError('territorio_para_string: argumento invalido')
    
    #caminhos verticais e caminhos horizontais
    
    caminhos_verticais = len(t)
    caminhos_horizontais = len(t[0])
    if caminhos_horizontais < 10 and caminhos_horizontais > 0:
        territorio = "  " 
    #escrever a primeira linha das letras
        for letra in range(caminhos_verticais):
            numero_letra = 65 + letra
            territorio += " " + chr(numero_letra) 

        territorio += "\n" 
    #loop inverso para dar como primeira linha a cintar de cima a linha representada pelo numero 4
        for numero in range(caminhos_horizontais, 0, -1):
            territorio += " " + str(numero)
            for letra in range(caminhos_verticais):
                if t[letra][numero-1] == 1:
                    territorio += " X" 
                else:
                    territorio += " ." 
            
            territorio += "  " + str(numero)   
            territorio += "\n"

        territorio += "  "
        for letra in range(caminhos_verticais):
            numero_letra = 65 + letra
            territorio += " " + chr(numero_letra) 
    

    elif caminhos_horizontais > 10:
        territorio = "  " 
    #escrever a primeira linha das letras
        for letra in range(caminhos_verticais):
            numero_letra = 65 + letra
            territorio += " " + chr(numero_letra) 

        territorio += "\n" 
    #loop inverso para dar como primeira linha a cintar de cima a linha representada pelo numero 4
        for numero in range(caminhos_horizontais, 0, -1):
            if numero >= 10:
                territorio += str(numero)
                for letra in range(caminhos_verticais):
                    if t[letra][numero-1] == 1:
                        territorio += " X"   
                    else:
                        territorio += " ." 
                territorio += " "
                territorio += str(numero)
                territorio += "\n"

            elif numero < 10:
                territorio += " " + str(numero)
                for letra in range(caminhos_verticais):
                    if t[letra][numero-1] == 1:
                        territorio += " X"  
                    else:
                        territorio += " ." 
            
                territorio += "  " + str(numero)   
                territorio += "\n"

        territorio += "  "
        for letra in range(caminhos_verticais):
            numero_letra = 65 + letra
            territorio += " " + chr(numero_letra) 
    
    else:
        territorio = " " 
    #escrever a primeira linha das letras
        for letra in range(caminhos_verticais):
            numero_letra = 65 + letra
            territorio += "  " + chr(numero_letra) 

        territorio += "\n" 
    #loop inverso para dar como primeira linha a cintar de cima a linha representada pelo numero 4
        for numero in range(caminhos_horizontais, 0, -1):
            if numero >= 10:
                territorio += str(numero)
                for letra in range(caminhos_verticais):
                    if t[letra][numero-1] == 1:
                        territorio += " X"  
                    else:
                        territorio += " ."
            
            elif numero < 10:
                territorio += " " + str(numero)
                for letra in range(caminhos_verticais):
                    if t[letra][numero-1] == 1:
                        territorio += " X"  + " "
                    else:
                        territorio += " ." + " "
            
            territorio += " " + str(numero)   
            territorio += "\n"

        territorio += "  "
        for letra in range(caminhos_verticais):
            numero_letra = 65 + letra
            territorio += " " + chr(numero_letra) 
        
    return territorio

"""
Esta fucnao vai nos dar todas as montanhas ou intersecoes livres que esta ligadas entre si 
"""
def obtem_cadeia(t, i):
    if eh_intersecao_valida(t, i) == False or eh_territorio(t) == False:
        raise ValueError('obtem_cadeia: argumentos invalidos')
    lista_cadeia = [i]
    a = 0
    intersecoes_adjacentes = list(obtem_intersecoes_adjacentes(t, i))
    #Caso em que a cadeia vai ser de montanhas 
    if eh_intersecao_livre(t, i) == False:
        while len(intersecoes_adjacentes) > a:
            for intersecao in intersecoes_adjacentes:
                if eh_intersecao_livre(t, intersecao) == False and intersecao not in lista_cadeia:
                    lista_cadeia.append(intersecao)
                    b = obtem_intersecoes_adjacentes(t, intersecao) 
                    for adjacente in b:
                        if adjacente not in intersecoes_adjacentes:
                            intersecoes_adjacentes.append(adjacente) 
            a += 1
    #Caso em que a cadeia vai ser de intersecoes livres       
    else:
        while len(intersecoes_adjacentes) > a:
            for intersecao in intersecoes_adjacentes:
                if eh_intersecao_livre(t, intersecao) == True and intersecao not in lista_cadeia:
                    lista_cadeia.append(intersecao)
                    b = obtem_intersecoes_adjacentes(t, intersecao) 
                    for adjacente in b:
                        if adjacente not in intersecoes_adjacentes:
                            intersecoes_adjacentes.append(adjacente)
            a += 1

    lista_cadeia_ordenada = ordena_intersecoes(lista_cadeia)
    return (lista_cadeia_ordenada)

"""
Esta funcao vai nos dar todos os vales, ou seja, as intersecoes livres que sao adjacentes a montanhas, de uma cadeia de montanhas
"""
def obtem_vale(t, i):
    if eh_intersecao_valida(t, i) == False or eh_intersecao_livre(t, i) == True:
        raise ValueError('obtem_vale: argumentos invalidos')
    vales = ()
#Ter a cadeia de montanhas de i    
    cadeia_montanhas = obtem_cadeia(t, i)
    for montanha in cadeia_montanhas:
#Obter todas as intersecoes adjacentes de todas as montanhas da cadeia de montanhas
        intersecoes_adjacentes = obtem_intersecoes_adjacentes(t, montanha)
        for adjacente in intersecoes_adjacentes:
#Verificar se as intersecoes adjacentes sao livres
            if eh_intersecao_livre(t, adjacente) and adjacente not in vales:
                vales += (adjacente,)

    return ordena_intersecoes(vales)

"""
Esta funcao vai verificar se duas intersecoes estao ligadas por uma cadeia
"""
def verifica_conexao(t, a, b):
    
    if eh_territorio(t) == False:
        raise ValueError('verifica_conexao: argumentos invalidos')
    
    if eh_intersecao_valida(t, a) == False or eh_intersecao_valida(t, b) == False:
        raise ValueError('verifica_conexao: argumentos invalidos')

#Se b estiver na cadeia de b entao as intersecoes estao conectadas
    if a in obtem_cadeia(t, b):
        return True
    else: 
        return False     

"""
Esta fucnao vai-nos dizer quantas montanhas há num territótrio
"""

def calcula_numero_montanhas(t):
    if eh_territorio(t) == False:
        raise ValueError('calcula_numero_montanhas: argumento invalido')
    numero_montanhas = 0
    #Entrar dentro de t para os seus tuplos
    for tuplos in t:
    #verificar se os elementos dos tuplos de t sao iguais a 1 
        for elementos in tuplos:
            if elementos == 1:
                numero_montanhas += 1
    
    return numero_montanhas

"""
Esta funcao vai-nos dizer quantas cadeias de montanhas diferentes há
"""

def calcula_numero_cadeias_montanhas(t):
    if eh_territorio(t) == False:
        raise ValueError('calcula_numero_cadeias_montanhas: argumento invalido')
    cadeias = []
    tamanho_tuplo = len(t[0])
    for i in range(len(t)):
        for j in range(tamanho_tuplo):
#Obter a cadeia de todas as montanhas do territorio
            if t[i][j] == 1:
                letra = chr(65+i)
                cadeia = (obtem_cadeia(t, (letra, j+1)))
                if cadeia not in cadeias:
                    cadeias.append(cadeia)          
    return len(cadeias)

"""
Esta funcao vai-nos dizer quantos vales há num território
"""

def calcula_tamanho_vales(t):
    if eh_territorio(t) == False:
        raise ValueError('calcula_tamanho_vales: argumento invalido')
    vales = []
    tamanho_tuplo = len(t[0])
    for i in range(len(t)):
        letra = chr(65+i)
        for j in range(tamanho_tuplo):
            if t[i][j] == 1:
#Obter a cadeia de montanhas de todas as montanhas do territorio 
                cadeia = obtem_cadeia(t, (letra, j+1))
                for intersecao in cadeia:
#Adicionar a variavel intersecoes_adjacentes todas as intersecoes adjacentes as intersecoes da cadeia
                    intersecoes_adjacentes = obtem_intersecoes_adjacentes(t, intersecao)
                    for intersecao_adjacente in intersecoes_adjacentes:
#Se a intersecao for livre adicionar a variavel vales    
                        if eh_intersecao_livre(t, intersecao_adjacente) and intersecao_adjacente not in vales:
                            vales.append(intersecao_adjacente)
    
    return len(vales)
