% Définir la plage des fichiers à traiter
startIndex = 12;
endIndex = 129;

% Boucle sur chaque fichier dans la plage
for i = startIndex:endIndex
    % Créer le nom de fichier en fonction de l'index
    inputFileName = sprintf('spectro_%d.txt', i);
    outputFileName = sprintf('spectro_%d.csv', i);
    
    % Vérifier si le fichier existe
    if exist(inputFileName, 'file') == 2
        % Lire les données avec un séparateur d'espace
        data = importdata(inputFileName, ' ');
        
        % Vérification des dimensions de la matrice
        [rows, cols] = size(data);
        fprintf('Le fichier %s contient une matrice de taille %d x %d.\n', inputFileName, rows, cols);
        
        % Écrire les données dans un fichier CSV
        dlmwrite(outputFileName, data, 'delimiter', ',');
        
        % Message de confirmation
        fprintf('Fichier %s converti en %s\n', inputFileName, outputFileName);
    else
        fprintf('Fichier %s non trouvé.\n', inputFileName);
    end
end

disp('Conversion terminée.');
