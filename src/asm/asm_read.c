/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   asm_read.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: prastoin <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/03/14 11:34:39 by prastoin          #+#    #+#             */
/*   Updated: 2019/04/26 16:10:18 by prastoin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_string.h"
#include "asm.h"
#include <fcntl.h>
#include <limits.h>

char			*change_ext(char *name)
{
	static char	file[PATH_MAX - 1];
	char		*dot;

	if (!(dot = ft_strrchr(name, '.')))
		return (NULL);
	dot = name + ft_strlen(name);
	if ((dot - name + (sizeof(EXT) - 1)) > PATH_MAX)
		return (NULL);
	ft_memcpy(file, name, dot - name);
	ft_memcpy(file + (dot - name), EXT, sizeof(EXT));
	return (file);
}

bool	ft_header(t_write *out, t_read *in)
{
	t_header		head;

	head = (t_header) {
		.size = 0
	};
	if (!asm_parse_header(in, &head))
		return (false);
	write_header(&head, out);
	return (true);
}

void		read_fixed(t_read *in, char *name)
{
	uint8_t	buffer[CHAMP_MAX_SIZE + HEADER_SIZE];
	t_write	out;
	t_hashtable		*table;

	out = init_write();
	out.buffer_size = CHAMP_MAX_SIZE + HEADER_SIZE;
	out.buffer = buffer;
	table = create_hashtable(8);
	if (!asm_parser(&out, in, table))
		return ;
	out.fd = open(name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	write (out.fd, out.buffer, out.nbr_write);
	ft_putf("done\n");
	return ;
}

void		read_streaming(t_read *in, char *name)
{
	uint8_t		buffer[BUFFER_SIZE];
	t_write		out;
	t_hashtable		*table;

	out = init_write();
	out.buffer_size = BUFFER_SIZE;
	out.flushable = true;
	out.buffer = buffer;
	out.fd = open(name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	table = create_hashtable(8);
	asm_parser(&out, in, table);
	printf("fin\n");
}

int main(int argc, char *argv[])
{
	t_flag	flag;
	char	*file;
	char	*out;
	int		fd;
	ssize_t ret;
	const t_arg args[] = {
		{ARG_BOOLEAN, 's', "Streaming", &flag.streaming, "Streaming on mode with this flag"},
		{ARG_BOOLEAN, 'w', "Werror", &flag.werror, "Warnings become errors"},
		{ARG_END, 0, 0, 0, 0}
	};
	t_read	in;

	flag = (t_flag) {};
	if ((ret = parse_args(args, argc, argv)) < 0)
		return (-1);
	if (!(out = change_ext(file)))
		return (ft_putf("Invalid name file\n"));
	if ((fd = open(file, O_RDONLY)) == -1)
		return (ft_putf("Open failed\n"));
	in = init_read(fd, file);
	(flag.streaming ? read_streaming : read_fixed)(&in, out);
}
