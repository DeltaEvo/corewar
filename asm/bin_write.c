/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   asm_write.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: prastoin <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/03/19 11:24:17 by prastoin          #+#    #+#             */
/*   Updated: 2019/03/26 18:08:22 by prastoin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "asm.h"
#include "ft_string.h"

void	io_flush(t_write *out)
{
	printf("Je flush\n");
	printf("%s\n", out->buffer);
	if (out->flushable == true)
		write(out->fd, out->buffer, out->index);
	else
	{
		printf("ERROR\n");
		out->buffer_size = -1;
		exit(0);
	}
	out->index = 0;
}

void	io_write(t_write *out, char *data, size_t size)
{
	size_t space;

	space = out->buffer_size - out->index;
	while (space < size)
	{
		ft_memcpy(out->buffer + out->index, data, space);
		io_flush(out);
		data += space;
		size -= space;
		out->nbr_write += space;
		space = out->buffer_size;
	}
	ft_memcpy(out->buffer + out->index, data, size);
	out->index += size;
	out->nbr_write += size;
}

void	io_write_one(t_write *out, char c)
{
	io_write(out, &c, 1);
}


void	io_write_int(t_write *out, uintmax_t nb, size_t nb_bytes)
{
	while (nb_bytes != 0)
	{
		nb_bytes--;
		io_write_one(out, (nb >> (nb_bytes * 8)) & 0xFF);
	}
}

void		bin_write_inst(t_write *out, t_instruction *inst, uint8_t last_label)
{
	size_t			i;
	uint8_t			ocp;

	ocp = last_label & 0b11;
	i = 0;
	io_write_int(out, inst->opcode, 1);
	while (g_ops[inst->opcode].params[i])
	{
		if (inst->params[i].type == PARAM_DIRECT)
			ocp |= 0b10 << ((3 - i) * 2);
		else if (inst->params[i].type == PARAM_INDIRECT)
			ocp |= 0b11 << ((3 - i) * 2);
		else if (inst->params[i].type == PARAM_REGISTER)
			ocp |= 0b01 << ((3 - i) * 2);
		i++;
	}
	if (i > 1)
		io_write_int(out, ocp, 1);
	i = 0;
	while (g_ops[inst->opcode].params[i])
	{

		if (inst->params[i].type == PARAM_DIRECT)
			io_write_int(out, inst->params[i].offset.offset, g_ops[inst->opcode].params[i] & PARAM_INDEX ? 2 : 4);
		else if (inst->params[i].type == PARAM_INDIRECT)
			io_write_int(out, inst->params[i].offset.offset, 2);
		else if (inst->params[i].type == PARAM_REGISTER)
			io_write_int(out, inst->params[i].reg.reg, 1);
		i++;
	}
}

void		io_seek(t_write *out, ssize_t offset, bool set_cur)
{
	if (out->flushable)
		lseek(out->fd, offset, (set_cur ? SEEK_SET : SEEK_CUR));
	else
		if (set_cur)
			out->index = offset;
		else
			out->index -= offset;
}

void		io_write_read(t_write *out, uint8_t *tmp, size_t size)
{
	if (out->flushable)
		read(out->fd, tmp, size);
	else
	{
		ft_memcpy(tmp, out->buffer + out->index, size);
		out->index += size;
	}
}

void	bin_write_end(t_write *out)
{
	size_t		len;
	t_header	*head;

	if (out->flushable)
		io_flush(out);
	len = 4 + sizeof(head->name) + 4 - sizeof(head->name) % 4;
	io_seek(out, len, true);
	len += 4 + sizeof(head->comment) + 4 - sizeof(head->comment) % 4;
	io_write_int(out, out->nbr_write - len, 4);
	if (out->flushable)
		io_flush(out);
}

bool	write_header(t_header *head, t_write *out)
{
	io_write_int(out, COREWAR_EXEC_MAGIC, 4);
	io_write(out, head->name, sizeof(head->name));
	io_write(out, (char [4]){0, 0, 0, 0}, 4 - sizeof(head->name) % 4);
	io_write_int(out, 0, 4);
	io_write(out, head->comment, sizeof(head->comment));
	io_write(out, (char [4]){0, 0, 0, 0}, 4 - sizeof(head->comment) % 4);
	return (true);
}

void		bin_resolve_label(t_write *out, size_t offset)
{
	ssize_t	src;
	uint8_t	opcode;
	uint8_t	ocp;
	uint8_t	tmp[4];
	int8_t	size;
	size_t	i;
	size_t save_ocp;
	size_t save_off;

	if (out->flushable)
		io_flush(out);
	src = out->nbr_write;
	while (offset != 0x0000)
	{
		io_seek(out, offset, true);
		io_write_read(out, &opcode, 1);
		if (g_ops[opcode].params[1])
		{
			size = 0;
			io_write_read(out, &ocp, 1);
			i = 0;
			while (i < (ocp & 0b11) && i < 2)
			{
				uint8_t type = (ocp >> ((3 - i) * 2)) & 0b11;
				if (type == 0b01)
					size += 1;
				else if (type == 0b10)
					size += g_ops[opcode].params[i] & PARAM_INDEX ? 2 : 4;
				else if (type == 0b11)
					size += 2;
				i++;
			}
			save_ocp = ocp;
			ocp &= 0b11111100;
			io_seek(out, -1, false);
			write(out->fd, &ocp, 1);
			io_seek(out, size, false);
			size = 2;
			uint8_t type = (ocp >> ((3 - i) * 2)) & 0b11;
			if (type == 0b10 && (!(g_ops[opcode].params[i] & PARAM_INDEX)))
				size = 4;
		}
		else
		{
			size = 2;
			if (g_ops[opcode].params[0] & PARAM_DIRECT && (!(g_ops[opcode].params[0] & PARAM_INDEX)))
				size = 4;
		}
		io_write_read(out, tmp, size);
		io_seek(out, -size, false);
		io_write_int(out, src - (ssize_t)offset, size);
		if (out->flushable)
			io_flush(out);
		save_off = offset;
		if (size == 2)
			offset = tmp[0] * 0x100 + tmp[1];
		else if (size == 4)
			offset = tmp[0] * 0x1000000 + tmp[1] * 0x10000 + tmp[2] * 0x100 + tmp[3];
		if (save_off == offset)
		{
			io_seek(out, (save_off + 1), true);
			if ((save_ocp & 0b11) >= 2)
				save_ocp -= 2;
			else
				save_ocp -= 1;
			ocp = save_ocp;
			write(out->fd, &ocp, 1);
		}
	}
	io_seek(out, src, true);
//	io_seek (out->fd, 0, SEEK_END); TODO David need to work
	out->nbr_write = src;
}
