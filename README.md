# Relatório de desenvolvimento do Trabalho 1: Consultas e Respostas de DNS

Com o objetivo de enviar uma consulta DNS, certas decisões foram tomadas. 
Utilizou-se uma struct para cada parte da mensagem DNS (cabeçalho, pergunta e domínio a ser retornado o IP), com respectivos campos, para facilitar o entendimento do código e agrupar os campos da mensagem.

Ao executar o arquivo, é necessário informar ao menos o domínio a ser convertido. O segundo parâmetro, que é o IP do servidor é opcional (com valor padrão de 127.0.0.1). Ainda no main, é chamada a função “encontrarIP”, que recebe o domínio a ser convertido, o tipo de requisição e o IP do servidor.

Na função “encontrarIP”, é criado um socket UDP e o servidor destino (struct sockaddr_in) é configurado para receber na porta 53. Em um array de char é sendo escrito o cabeçalho, o site a ser convertido em formato DNS. O formato DNS de um domínio é escrever a quantidade de caracteres até o próximo “.”, eliminando todos os “.” no processo. Ex: “www.google.com” vira “3www6google3com”.

Ao escrever na mensagem a ser enviada ao servidor é escrito, também, o tipo de requisição. Finalmente, a mensagem é enviada ao servidor. O tempo que se espera para retornar uma resposta é de 5 segundos.

Caso não se receba nada, o programa exibe uma mensagem de erro (“Falhou em receber uma resposta”) e é encerrado. Caso contrário, é lida é convertida a resposta retornada pelo servidor.

Após a leitura, é impresso o domínio digitado e, na linha seguinte, a resposta do servidor. (Um IPv4 para uma requisição do tipo A e um IPv6 para uma tipo AAAA. Não conseguimos traduzir a resposta do tipo MX, que contém um mail exchange)

Embora uma requisição possa retornar mais de uma resposta, no programa é tratada somente uma (primeira) resposta.

Esse processo é repetido 3 vezes (exceto quando a resposta não é retornada em até 5 segundos, porque, nesse caso, o programa foi encerrado) para cada tipo de requisição (A, MX, AAAA).

Ao usar o Wireshark, é possível ver os endereços retornados do domínio solicitado, baseado no tipo de requisição passado na função “encontrarIP”.
