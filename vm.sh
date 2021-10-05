#!/usr/bin/env bash

host_ssh_port=4242

iso_url="https://projects.intra.42.fr/uploads/document/document/5137/SnowCrash.iso"

vm_dir="$PWD"

vm_os="Linux_64"
vm_image="$vm_dir/$(basename "$iso_url")"
vm_name="SnowCrash"
vm_ram="1024"
vm_vram="0"
vm_gfx="none"
vm_net="nat"

vm_ssh_port="4242"

print_help()
{
	echo -e "Usage $0 [command]
Commands:
    up      Setup and stat the virtual machine.
    down    Unregister and delete the virtual machine.
    ip      Show a running virtual machine's IPV4 address.
    ssh     Connect to the virtual machine using SSH.
    help    Show this help message.

If no argument is provided, 'up' will be assumed."
}

print_ssh_usage()
{
	echo -e "Usage: $0 ssh user[:pass]"
}

print_vm_stopped()
{
	echo -e "'$vm_name' is not running!

Use '$0 up' to start it up." 1>&2
}

print_vm_started()
{
	echo -e "'$vm_name' was already started!

Use '$0 ssh' to connect."
}

vm_exists()
{
	VBoxManage showvminfo "$vm_name" > /dev/null 2>&1
}

vm_running()
{
	VBoxManage list runningvms | grep "\"$vm_name\"" > /dev/null
}

vm_up()
{
	if ! vm_exists
	then
		# Download the disk image.
		pushd "$(dirname "$vm_image")"
			while ! md5sum --check md5sums
			do
				echo "Fetching '$vm_image'..."

				curl -L -O -C - "$iso_url" --output "$vm_image"
			done
		popd

		echo "Initializing '$vm_name' at '$vm_dir'..."

		# Create and register vm in current working directory.
		VBoxManage createvm --name "$vm_name" --ostype "$vm_type" --register --basefolder "$vm_dir"
		VBoxManage modifyvm "$vm_name" --memory "$vm_ram" --vram "$vm_vram" --graphicscontroller "$vm_gfx" --nic1 "$vm_net" --natpf1 "ssh,tcp,,$vm_ssh_port,,$host_ssh_port"

		# Add an IDE controller
		VBoxManage storagectl "$vm_name" --name "IDE Controller" --add ide --controller PIIX4

		# Attach the disk image.
		VBoxManage storageattach "$vm_name" --storagectl "IDE Controller" --port 0 --device 0 --type dvddrive --medium "$vm_image"
	fi

	if ! vm_running
	then
		echo "Starting '$vm_name'..."
		VBoxManage startvm "$vm_name" --type headless
	else
		print_vm_started 2>&1
		return 1
	fi
}

vm_down()
{
	if vm_exists
	then
		if vm_running
		then
			echo "Waiting for '$vm_name' to power off,,,"
			VBoxManage controlvm "$vm_name" poweroff && sleep 2
		fi
		echo "Tearing down '$vm_name'..."
		VBoxManage unregistervm "$vm_name" --delete
	fi
}

vm_ipv4()
{
	if [ "$vm_net" = nat ]
	then
		echo "localhost"
	elif vm_running
	then
		VBoxManage guestproperty get "$vm_name" /VirtualBox/GuestInfo/Net/0/V4/IP | cut -d' ' -f2
	else
		print_vm_stopped 2>&1
		return 1
	fi
}

vm_ssh() # user
{
	local user="$1"

	if [ -z $user ]
	then
		echo "$0: ssh: Missing user argument." 2>&1
		print_ssh_usage 2>&1
		return 1
	fi

	if vm_running
	then
		if [ "$vm_net" = nat ]
		then
			local port="$host_ssh_port"
		else
			local port="$vm_ssh_port"
		fi

		local pass="${user##*:}"

		if ! [ -z $pass ]
		then
			"$(dirname "$0")/utils/pass.exp" "$pass" ssh -p "$port" "${user%%:*}@$(vm_ipv4)"
		else
			ssh -p "$port" "$user@$(vm_ipv4)"
		fi
	else
		print_vm_stopped 2>&1
		return 1
	fi
}

case "$1" in
	"up" | ""	)	vm_up;;
	"down"		)	vm_down;;
	"ip"		)	vm_ipv4;;
	"ssh"		)	shift; vm_ssh $@;;
	"help"		)	print_help;;
	*			)	print_help && exit 1;;
esac
