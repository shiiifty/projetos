# Variáveis
CXX = g++
CXXFLAGS = -std=c++17 -g
USER_INC = -Iclient -Icommon
ES_INC = -Iserver -Icommon

# Ficheiros objeto para o executável 'user'
USER_OBJS = client/commands.o \
			client/user.o \
			common/common.o \

# Ficheiros objeto para o executável 'ES'
ES_OBJS = server/es.o \
		common/common.o \

# Alvo principal: 'all' compila ambos os executáveis
all: user ES

# Regra para compilar o executável 'user'
user: $(USER_OBJS)
	$(CXX) $(CXXFLAGS) $(USER_OBJS) $(USER_INC) -o user

# Regra para compilar o executável 'ES'
ES: $(ES_OBJS)
	$(CXX) $(CXXFLAGS) $(ES_OBJS) $(ES_INC) -o ES

# --- Regras de Compilação de Objetos ---
# Regra genérica para compilar ficheiros .cpp em .o para os ficheiros 'user'
client/%.o: client/%.cpp
	$(CXX) $(CXXFLAGS) $(USER_INC) -c $< -o $@

# Regra genérica para compilar ficheiros .cpp em .o para os ficheiros 'ES' (no diretório server)
server/%.o: server/%.cpp
	$(CXX) $(CXXFLAGS) $(ES_INC) -c $< -o $@

# Regra para os ficheiros em 'common' (usados por ambos)
common/%.o: common/%.cpp
	$(CXX) $(CXXFLAGS) $(USER_INC) -c $< -o $@


# Alvo para limpar os ficheiros gerados
clean:
	rm -f user ES $(USER_OBJS) $(ES_OBJS)