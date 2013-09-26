nodes = 10000
edges = 9990000

puts nodes
puts edges

edgeHash=Hash.new

edges.times do
	from,to,weight=0,0,0
	begin 
		from,to,weight=rand(nodes),rand(nodes),rand(edges)+1
	end while from==to or edgeHash.has_key?("#{from}-#{to}:#{weight}")
	edgeHash["#{from}-#{to}:#{weight}"]=1

	puts "#{from} #{to} #{weight}"
end