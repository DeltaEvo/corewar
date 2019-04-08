/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_ocp_values.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: prastoin <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/04/03 15:21:34 by prastoin          #+#    #+#             */
/*   Updated: 2019/04/08 16:13:34 by prastoin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "vm.h"

void		conv_int_to_bin(size_t nbr, uint8_t value[REG_SIZE])
{
	ssize_t i;

	i = REG_SIZE - 1;
	while (i >= 0)
	{
		value[i] = nbr % 255;
		nbr /= 255;
		i--;
	}
}

bool		ft_check_ocp(uint8_t ocp, uint8_t opcode)
{
	size_t i;
	size_t type;

	i = 0;
	while (g_ops[opcode].params[i])
	{
		type = (ocp >> (3 - i) * 2) & 0b11;
		if (type == 0b11)
		{
			if (!(g_ops[opcode].params[i] & 0b10))
			{
				return (false);
			}
		}
		else if (type == 0b10)
		{
			if (!(g_ops[opcode].params[i] & 0b1))
			{
				return (false);
			}
		}
		else if (type == 0b01)
		{
			if (!(g_ops[opcode].params[i] & 0b100))
			{
				return (false);
			}
		}
		else
		{
			return (false);
		}
		i++;
	}
	return (true);
}

bool		ft_get_value(ssize_t nbr, uint8_t type, t_process *process, t_vm *vm)
{
	if (type == OCP_REG)
	{
		if (nbr >= 16)
			return (false);
		else
			ft_memcpy(process->tampon, process->registre[nbr], REG_SIZE);
	}
	else if (type == OCP_DIR)
		conv_int_to_bin(nbr, process->tampon);
	else if (type == OCP_IND)
		mem_read(vm->mem, process->tampon, (process->offset + nbr) % MEM_SIZE, 2);
	else
		return (false);
	return (true);
}

bool		ft_get_value_mod(ssize_t nbr, uint8_t type, t_process *process, t_vm *vm)
{
	if (type == OCP_REG)
	{
		if (nbr >= 16)
		{
			printf("\033[31m get value failed bad register\033[0m ");
			return (false);
		}
		else
			ft_memcpy(process->tampon, process->registre[nbr], REG_SIZE);
	}
	else if (type == OCP_DIR)
		conv_int_to_bin(nbr, process->tampon);
	else if (type == OCP_IND)
		mem_read(vm->mem, process->tampon, (process->offset + nbr % IDX_MOD) % MEM_SIZE, 2);
	else
		return (false);
	return (true);
}
