#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h> 

typedef struct	json {
	enum {
		MAP,
		INTEGER,
		STRING
	} type;
	union {
		struct {
			struct pair	*data;
			size_t		size;
		} map;
		int	integer;
		char	*string;
	};
}	json;

typedef struct	pair {
	char	*key;
	json	value;
}	pair;

void	free_json(json j);
int	argo(json *dst, FILE *stream);
int parsing(json *dst, FILE *stream);

int	peek(FILE *stream)
{
	int	c = getc(stream);
	ungetc(c, stream);
	return c;
}

void	unexpected(FILE *stream)
{
	if (peek(stream) != EOF)
		printf("unexpected token '%c'\n", peek(stream));
	else
		printf("unexpected end of input\n");
}

int	accept(FILE *stream, char c)
{
	if (peek(stream) == c)
	{
		(void)getc(stream);
		return 1;
	}
	return 0;
}

int	expect(FILE *stream, char c)
{
	if (accept(stream, c))
		return 1;
	unexpected(stream);
	return 0;
}

void	free_json(json j)
{
	switch (j.type)
	{
		case MAP:
			for (size_t i = 0; i < j.map.size; i++)
			{
				free(j.map.data[i].key);
				free_json(j.map.data[i].value);
			}
			free(j.map.data);
			break ;
		case STRING:
			free(j.string);
			break ;
		default:
			break ;
	}
}

void	serialize(json j)
{
	switch (j.type)
	{
		case INTEGER:
			printf("%d", j.integer);
			break ;
		case STRING:
			putchar('"');
			for (int i = 0; j.string[i]; i++)
			{
				if (j.string[i] == '\\' || j.string[i] == '"')
					putchar('\\');
				putchar(j.string[i]);
			}
			putchar('"');
			break ;
		case MAP:
			putchar('{');
			for (size_t i = 0; i < j.map.size; i++)
			{
				if (i != 0)
					putchar(',');
				serialize((json){.type = STRING, .string = j.map.data[i].key});
				putchar(':');
				serialize(j.map.data[i].value);
			}
			putchar('}');
			break ;
	}
}

int parse_str(json *dst, FILE *stream)
{
	char *str;
	int i;

	str = malloc(1 * sizeof(char));
	if (!str)
		return -1;

	i = 0;
	while (accept(stream, '"') != 1)
	{
		if (peek(stream) == EOF) 
		{
			unexpected(stream);
			free(str);
			return -1;
		}
		if (accept(stream, '\\') == 1)
		{
			if (peek(stream) != '\\' && peek(stream) != '"')
			{
				unexpected(stream);
				free(str);
				return -1;
			}
		}
		str[i] = getc(stream);
		i++;
		str = realloc(str, (i + 1) * sizeof(char));
		if (!str)
		{
			free(str);
			return -1;
		}
	}
	str[i] = '\0';
	dst->type = STRING;
	dst->string = str;
	return 1;
}

int parse_int(json *dst, FILE *stream)
{
	int num;

	if (!(fscanf(stream, "%d" , &num)))
	{
		unexpected(stream);
		return -1;
	}
	dst->type = INTEGER;
	dst->integer = num;
	return 1;
}

int parse_map(json *dst, FILE *stream)
{
	json get_key;
	json get_value;
	pair *temp_data;
	int size = 0;
	int i;

	temp_data = malloc(1 * sizeof(pair));
	if (!temp_data)
		return -1;
	while (accept(stream, '}') != 1)
	{
		if (accept(stream, '"') == 1)
		{
			if (parse_str(&get_key, stream) != 1)
				goto error;
			temp_data[size].key = get_key.string;
		}
		else
		{
			unexpected(stream);
			goto error;
		}
		if (accept(stream, ':') != 1)
		{	
			unexpected(stream);
			free(get_key.string);
			goto error;
		}
		if (parsing(&get_value, stream) != 1)
		{
			free(get_key.string);
			goto error;
		}
		temp_data[size].value = get_value;
		size++;
		if (accept(stream, ',') != 1)
		{
			if (accept(stream, '}') == 1)
				break;
			unexpected(stream);
			goto error; 
		}
		temp_data = realloc(temp_data, (size + 1) * sizeof(pair));
		if (!temp_data)
			goto error;
	}
	dst->type = MAP;
	dst->map.data = temp_data;
	dst->map.size = size;	
	return 1;

error:
	i = 0;
	while (i < size)
	{
		free(temp_data[i].key);
		free_json(temp_data[i].value);
		i++;
	}
	free(temp_data);
	return -1;
}

int parsing(json *dst, FILE *stream)
{
	if (accept(stream, '"') == 1)
	{
		return(parse_str(dst, stream));
	}
	else if (isdigit(peek(stream)) || peek(stream) == '-')
	{
		return(parse_int(dst, stream));
	}
	else if (accept(stream, '{') == 1)
	{
		return(parse_map(dst, stream));
	}
	else
		unexpected(stream);
	return -1;
}

int	argo(json *dst, FILE *stream)
{
	if (parsing(dst, stream) == 1)
	{
		if (peek(stream) != EOF)
		{
			unexpected(stream);
			return -1;
		}
	}
	else
		return -1;
	return 1;
}

int	main(int argc, char **argv)
{
	if (argc != 2)
		return 1;
	char *filename = argv[1];
	FILE *stream = fopen(filename, "r");
	json	file;
	if (argo (&file, stream) != 1)
	{
		free_json(file);
		// fclose(stream); // for valgrind
		return 1;
	}
	serialize(file);
	free_json(file);
	printf("\n");
	// fclose(stream); // for valgrind
}
